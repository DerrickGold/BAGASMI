#include <nds.h>
#include <filesystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <fat.h>
#include <sys/stat.h>
#include <unistd.h>

#include <nf_lib.h>

/*
ToDo:
	rewrite text buffering core (start with static, then dynamic)
	rewrite text displaying

*/

#define INITIAL_LINE_LEN 64
#define TERMINAL_BOTTOM ((SCREEN_HEIGHT/8) - 1)
PrintConsole debugOut;
PrintConsole topScreen;
/*====================================================================================
Standard functions


====================================================================================*/
//general console initialization
void NDS_InitConsoleTop(void){
    videoSetMode(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    consoleInit(&topScreen, 0,BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true);
    consoleSelect(&topScreen);
    consoleSetWindow (&topScreen, 0, 0, 32, 26);
}

//init keyboard for editor
void NDS_EditorInitKB(void){
	//init bottom screen
    videoSetModeSub(MODE_0_2D);
    vramSetBankC(VRAM_C_SUB_BG);

    //debug console
    consoleInit(&debugOut, 1,BgType_Text4bpp, BgSize_T_256x256, 18, 2, false, true);
    consoleSetWindow (&debugOut, 0, 0, 32, 15);
    consoleSelect(&debugOut);

    //init keyboard
	keyboardInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x512, 14, 0, false, true);
	keyboardShow();
}








/*====================================================================================
Text file loading, writing, editing


====================================================================================*/
typedef struct txtLine_s{
	char *str;
	int len;
}txtLine;

typedef struct txtFile_s{
	char *filePath;
	txtLine *line;
	int lineCount, usedLines;
}txtFile;


static void txt_freeLine(txtLine *line){
	if(!line) return;

	//if the line exists, free the data
	if(line->len > 0 && line->str){
		free(line->str);
	}
	//otherwise, initialize it
	line->str = NULL;
	line->len = 0;
}

//allocate a single line of text and copy data to it
static int txt_MakeLine(txtLine *line, const char *str){
	txt_freeLine(line);

	int len = INITIAL_LINE_LEN;
	int copyLen = 0;
	if(str){
		copyLen = strlen(str);
		while(copyLen > len) len += INITIAL_LINE_LEN;
	}

	//allocate line
	line->str = calloc(len, sizeof(char));
	if(!line->str){
		iprintf("Error allocating a line\n");
		return 0;
	}

	if(str && copyLen){
		strncpy(line->str, str, copyLen);
		line->len = copyLen;
	}

	return 1;
}

//double the size of an existing line
static int txt_ExpandLine(txtLine *line){
	iprintf("expanding line\n");
	char *tempStr = realloc(line->str, ((line->len + 1) << 1) * sizeof(char));
	if(!tempStr){
		iprintf("Error expanding line\n");
		return 0;
	}
	line->str = tempStr;
	//memset(&line->str[line->len + 1], 0, line->len);
	return 1;
}

//checks a line to see if we need to expand it
static int txt_CheckLineLen(txtLine *line){
	if((line->len + 1) % INITIAL_LINE_LEN == 0){
		return txt_ExpandLine(line);
	}
	return 0;
}

//adds an extra screen worth of buffers
static int txt_ExpandFile(txtFile *file){
	iprintf("expanding file: %d\n", file->lineCount);
	txtLine *tempLines = NULL;

	int oldLineCount = file->lineCount;
	int newLineCount = (file->lineCount << 1) + 1;

	tempLines = realloc(file->line, sizeof(txtLine) * newLineCount);
	if(!tempLines){
		iprintf("error expanding file\n");
		return -1;
	}
	file->line = tempLines;
	iprintf("file expanded, creating lines\n");

	int lineNum = 0;
	for(lineNum = oldLineCount; lineNum < newLineCount - 1; lineNum++){
		txtLine *curLine = &file->line[lineNum];
		curLine->str = NULL;
		curLine->len = 0;
		txt_MakeLine(curLine, NULL);
	}
	//if(!txt_MakeLine(&file->line[file->lineCount])){
	//	return -2;
	//}
	iprintf("line created\n");
	file->lineCount = newLineCount;
	return 1;
}


static int txt_InsertLine(txtFile *file, int posNum, int lineXPos){

	if(posNum > file->lineCount || posNum < 0) return 0;
	//if the line is empty, no need to insert anything
	if(!file->line[posNum].len) {
		iprintf("blank line!\n");
		return 1;
	}

	//otherwise we have to shift stuff down


	//expand file first if necessary
	if(file->usedLines >= file->lineCount){
		txt_ExpandFile(file);
	}

	//ToDo: FIX THIS
	//fix for when cursor is not at end of line to insert and copy text to new line
	//lineXPos should hold where on the line to insert the new line
	//then use file->line[pos].len - lineXPos to collect data size
	int lineNum = file->usedLines;


	while(lineNum > posNum){
		char *prevStr = file->line[lineNum - 1].str;
		iprintf("Moving string: %s\n", prevStr);
		txt_freeLine(&file->line[lineNum]);
		if(!txt_MakeLine(&file->line[lineNum], prevStr)){
			iprintf("error inserting lines\n");
			return -1;
		}
		iprintf("lineNum:%d\n", lineNum);
		lineNum--;
	}

	char *tempLine = NULL, tempLen = file->line[posNum - 1].len;
	//insert the line
	if(lineXPos < tempLen){
		tempLine = &file->line[posNum - 1].str[lineXPos];

	}
	int rtrn = txt_MakeLine(&file->line[posNum], tempLine);
	//clear end of line where newline is inserted
	if(tempLine){
		memset(tempLine, 0, (lineXPos - tempLen));
		file->line[posNum - 1].len = lineXPos;
	}


	return rtrn;
}

static int txt_DeleteLine(txtFile *file){

	file->lineCount--;
	return 0;
}

//cleans a text file
static void txt_CleanFile(txtFile *file){
	if(file->filePath){
		free(file->filePath);
	}
	file->filePath = NULL;

	if(file->line){
		int i = 0;
		for(i = 0; i < file->lineCount; i++){
			txt_freeLine(&file->line[i]);
		}

		free(file->line);
		file->line = 0;
	}

	file->lineCount = 0;
}

static int txt_WriteFile(txtFile *file){
	if(!file->filePath) return -1;

	//open the file
	FILE *temp = fopen(file->filePath, "w");
	if(!temp){
		iprintf("Failed to open file for writing\n");
		return -1;
	}

	//write each line to file
	int currentLine = 0;
	for(currentLine = 0; currentLine < file->lineCount; currentLine++){

		txtLine *line = &file->line[currentLine];
		if(line->str){
			fprintf(temp, "%s\n", line->str);
		} else {//just a blank line
			fprintf(temp, "\n");
		}
	}

	//close file
	fclose(temp);
	return 1;
}

static int txt_OpenFile(const char *filePath, txtFile *file){
	/*txt_CleanFile(file);

	//allocate and store filePath for later
	int pathLen = strlen(filePath);
	file->filePath = calloc(pathLen + 1, sizeof(char));
	if(!file->filePath){
		iprintf("failed to allocate file path\n");
		return 0;
	}

	strncpy(file->filePath, filePath, pathLen);

	//now to open the file
	FILE *inFile = fopen(filePath, "r");
	if(!inFile){
		iprintf("failed to open file for reading\n");
		return 0;
	}


	char lineBuf[512];
	char *temp = NULL;

	do{
		//grab current line in file
		temp = fgets(lineBuf, 512, inFile);

		//remove newline characters
		char *newLine = strrchr(lineBuf, '\n');
		if(newLine) *newLine = '\0';

		if(txt_ExpandFile(file) != 1){
			return -2;
		}




	}while(!feof(inFile) && temp != NULL);

	//close file
	fclose(inFile);*/
	return 1;
}

//Creates a whole screen worth of buffers
static int txt_NewFile(txtFile *file){
	iprintf("Making new file\n");
	file->filePath = NULL;
	file->line = NULL;
	file->lineCount = SCREEN_HEIGHT / 8;

	//insert one line
	//expand file first
	txtLine *tempLines = NULL;

	tempLines = realloc(file->line, sizeof(txtLine) * (file->lineCount + 1));
	if(!tempLines){
		iprintf("error creating new file\n");
		return -1;
	}
	file->line = tempLines;


	int lineNum = 0;
	for(; lineNum < file->lineCount; lineNum++){
		txtLine *curLine = &file->line[lineNum];
		curLine->str = NULL;
		curLine->len = 0;
		txt_MakeLine(curLine, NULL);
	}
	iprintf("alloc'd lines: %d\n", file->lineCount);
	file->lineCount++;
	return 1;
}

/*====================================================================================
Text file displaying, scrolling


====================================================================================*/
typedef struct Display_s{
	int scrollX, lineY, cursorX, cursorY;
	txtFile file;
}Display;



static void clearConsole(void){
	iprintf ("\x1b[2J");
}

static void setTextColor(int color){
	char outBuf[16];
	sprintf(outBuf, "\x1b[3%d;1m", color);
	iprintf(outBuf);
}

static int currentLine(Display *out){
	return out->lineY + out->cursorY;
}

txtLine *getCurrentLine(Display *out, int *lineNum){
	*lineNum = currentLine(out);
	return &out->file.line[*lineNum];
}


static void drawCursor(Display *out){
	setTextColor(1);
	char posBuf[16];
	sprintf(posBuf, "\x1b[%d;%dH#", out->cursorY, out->cursorX);
	iprintf(posBuf);
	setTextColor(7);
}




static void displayCheckScroll(Display *out){
	//if cursor hits top of screen, scroll up
	if(out->cursorY < 0){
		out->cursorY = 0;
		if(out->lineY > 0) out->lineY--;
	}
	//otherwise if cursor hits terminal bottom
	if(out->cursorY > TERMINAL_BOTTOM && currentLine(out) < out->file.usedLines)
	{
		out->cursorY = TERMINAL_BOTTOM;
	    out->lineY++;
	} else if(currentLine(out) >= out->file.usedLines){
		out->cursorY = TERMINAL_BOTTOM;
	}

	int lineNum = 0;
	txtLine *curLine = getCurrentLine(out, &lineNum);

	out->cursorX = curLine->len;
	iprintf("cursorY: %d, scroll: %d\n", out->cursorY, out->lineY);
	out->scrollX = 0;
}

static void cursorDown(Display *out){
	out->cursorY++;
	displayCheckScroll(out);
}

static void cursorUp(Display *out){
	out->cursorY--;
	displayCheckScroll(out);
}

static void cursorLeft(Display *out){
	out->cursorX--;
	displayCheckScroll(out);
}

static void cursorRight(Display *out){
	out->cursorX++;
	displayCheckScroll(out);
}

static void drawDisplay(Display *out){
	//iprintf("update console\n");
	consoleSelect(&topScreen);
	//clear the console to redraw it
	clearConsole();
	setTextColor(7);

	//since we are using console based text, text is tiled
	//so loop through each line that should be displayed and print
	//from the current x and y offsets
	for(int lineNum = out->lineY; lineNum <= out->lineY + (SCREEN_HEIGHT / 8) && lineNum <= out->file.usedLines; lineNum++){
		txtLine *curLine = &out->file.line[lineNum];

		//if we aren't scrolling too far for a line, we should be able to print it
		if(out->scrollX < curLine->len){
			//since we can only print 32 characters long
			char outBuf[SCREEN_WIDTH/8];
			strncpy(outBuf, &curLine->str[out->scrollX], SCREEN_WIDTH/8);
			iprintf("%s\n", outBuf);
		} else {
			iprintf("\n");
		}
	}
	drawCursor(out);
	consoleSelect(&debugOut);
}



void textEditor(const char *filePath){
	//our text file handle
	Display Text = {};

	//if no file path is specified, we create a new file
	if(!filePath){
		 txt_NewFile(&Text.file);
	} else { //otherwise open the file
		txt_OpenFile(filePath, &Text.file);
	}

	//init keyboard
	NDS_EditorInitKB();

	int lineNum = 0;
	txtLine *curLine = NULL;
	while(true){
		//update current line info
		curLine = getCurrentLine(&Text, &lineNum);

		int key = keyboardUpdate();
		if(key != NOKEY){

			switch(key){
				case DVK_ENTER:{
					curLine->str[curLine->len] = '\0';
					iprintf("currentLine: %d\n", lineNum);
					//adjust file size accordingly
					Text.file.usedLines++;
					if(Text.file.usedLines >= Text.file.lineCount - 1){
						txt_ExpandFile(&Text.file);
					} else {
						txt_InsertLine(&Text.file, lineNum + 1, Text.cursorX + Text.scrollX);
					}

					//check whether we need to expand or insert
					//greater than line count, expand file
					//if(lineNum + 1 >= Text.file.lineCount){

					//} else { //we are inserting
						//txt_InsertLine(&Text.file, lineNum + 1, lineBuf);
					//}

					//Move cursor down screen till it hits bottom
					//also updates the next line number
					if(Text.cursorY < TERMINAL_BOTTOM){
						iprintf("cursor down!\n");
						Text.cursorY++;
					} else if (Text.cursorY >= TERMINAL_BOTTOM){//then scroll the file up
						iprintf("scroll up!\n");
						Text.lineY++;
					}

					//reset x coords`
					Text.scrollX = 0;
					Text.cursorX = 0;

				}break;

				case DVK_UP:
					cursorUp(&Text);
				break;
				case DVK_DOWN:
					cursorDown(&Text);
				break;
				case DVK_RIGHT:
					cursorRight(&Text);
				break;
				case DVK_LEFT:
					cursorLeft(&Text);
				break;

				case '\b'://back space

				break;

				default:
					txt_CheckLineLen(curLine);
					curLine->str[curLine->len++] = key;
					curLine->str[curLine->len] = '\0';
					Text.cursorX++;//increase cursor position
				break;
			}
			drawDisplay(&Text);
		}

		swiWaitForVBlank();
	}
}
