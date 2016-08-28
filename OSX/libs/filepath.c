/*---------------------------------------------------------------------------------
 libBAG  Copyright (C) 2010 - 2013
  BassAceGold - <BassAceGold@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.

  3. Restricting the usage of or portions of code in this library
     from other software developers and or content creators
     is strictly prohibited in applications with software creation capabilities
     and or with the intent to profit from said restrictions.

  4. This notice may not be removed or altered from any source
     distribution.
---------------------------------------------------------------------------------*/

#include "filepath.h"
#include "../config.h"

//Todo: Store root drive info somewhere eg. C:

//clear a path entry string
static void _pathFreeStr(_pathStr *str){
    if(str->len > 0 && str->str){
        free(str->str);
    }
    str->str = NULL;
    str->len = 0;
    return;
}

//updates pointer to extension start
//Todo: Remove dependency on chdir for pc
static void _pathUpdateExt(FilePath *path){
    if(path->depth == 0) return;

    path->folder = 1;//assume path leads to folder till otherwise proven

    //build filepath to check
    char _tempPath[MAX_PATH];
    if(!FilePath_Export(path, _tempPath, MAX_PATH)){
    	//BAG_DBG_LibMsg("BAG: Error export path for ext check.");
    	return;
    }

    //check if path leads to file or directory
    struct stat st = {};
    stat(_tempPath, &st);

    //if it is a directory, make the extension nothing
    if(S_ISDIR(st.st_mode)){
    	path->ext = NULL;
    } 
    else {//otherwise, grab the extension
        //grab file extension (unicode safe, hopefully)
        char lvl = path->depth - 1;
        int len = path->level[lvl].len;
        char *str = path->level[lvl].str;

        while(len > 0 && str[len] != '.'){
            path->ext = (char*)&str[len];
            len--;
        }

        if(len <= 0) path->ext = NULL;
        else{
            //exclude the . from the extension
            len += (str[len] == '.');
        }
        path->folder = 0;
    }

}

//set a path entry string
static int _pathSetStr(_pathStr *str, const char *inStr){
    //_pathFreeStr(str);
    size_t len = strlen(inStr);
    if(len > 255){
        //printf("Error, path entry > 255\n");
        //BAG_DBG_LibMsg("BAG: Path Entry > 255 characters\n");
        return 0;
    }

    str->str = calloc(sizeof(char), len + 1);
    if(!str->str){
        //printf("error allocating path entry\n");
        //BAG_DBG_LibMsg(BUFFER_FAILED_MSG, len + 1, "File Path Entry");
        return 0;
    }
    str->len = len;
    memcpy(str->str, inStr, len);
    return 1;
}


//return the character length of a file path
int FilePath_Len(FilePath *path){
    int len = 0;
    for(int i = 0; i < path->depth; i++){
        len += path->level[i].len;
    }

    //then add room for separators
    len += path->depth + 1;
    return len;
}


void FilePath_Clean(FilePath *path){
    if(path->level && path->depth > 0){
	    for(int i = 0; i < path->depth; i++){
	        _pathFreeStr(&path->level[i]);
	    }
	    free(path->level);
	}
    path->level = NULL;
    path->depth = 0;
    return;
}

//add an entry to the file path
int FilePath_Add(FilePath *path, const char *entry){
	//if dealing with absolute paths, we can simplify them

	if(path->absolute){
	    //go up a directory
	    if(strncmp(entry, "..", strlen(entry))){
	        return FilePath_Remove(path);
	    }
	    //do nothing if adding the current directory
	    else if(strncmp(entry, ".", strlen(entry))){
	        return 1;
	    }
	}

	unsigned int size = sizeof(_pathStr) * (path->depth + 1);
    _pathStr *temp = realloc(path->level, size);
    if(!temp){
        //printf("error allocating extra path entry\n");
        //BAG_DBG_LibMsg(BUFFER_FAILED_MSG, size, "File Path Levels");
        return 0;
    }
    path->level = temp;

    int rtrn = _pathSetStr(&path->level[path->depth++], entry);
    //update file path
    _pathUpdateExt(path);

    //add the entry
    return rtrn;
}

//equivalent of going back one directory or path entry
int FilePath_Remove(FilePath *path){
    if(path->depth <= 0) return 1;

    path->depth--;
    _pathFreeStr(&path->level[path->depth]);

    unsigned int size = sizeof(_pathStr) * path->depth;
    _pathStr *temp = realloc(path->level, size);
    if(!temp){
        //BAG_DBG_LibMsg(BUFFER_FAILED_MSG, size, "File Path Shrink");
        return 0;
    }
    path->level = temp;

    //update file path
    _pathUpdateExt(path);

    return 1;
}

//grab a fully built path for an entry
int FilePath_ExportEx(FilePath *path, char *outBuf, size_t outBufLen, int depth){
    int pathLen = FilePath_Len(path);
    if(pathLen <= 0) return 1;

    if(depth < 0 || depth > path->depth){
        //BAG_DBG_LibMsg("BAG: File Path Depth out of bounds\n");
        return 0;
    }

    //first calculate length so we know if buffer has enough room
    if( pathLen > outBufLen){
        //printf("File path larger than buffer!\n");
        //BAG_DBG_LibMsg("BAG:File path > buffer size\n");
        return 0;
    }
    //make sure the buffer is cleared
    memset(outBuf, 0, outBufLen);

    //ToDo: Make unicode safe
    //then we can build the path
    if(path->absolute) strcat(outBuf, DIR_ROOT);
    int i = 0;
    for(; i < depth - 1; i++){
        strncat(outBuf, path->level[i].str, path->level[i].len);
        strcat(outBuf, DIR_SEPARATOR_STR);
    }
    //finally append the last entry
    strncat(outBuf, path->level[i].str, path->level[i].len);
    return 1;
}

int FilePath_Export(FilePath *path, char *outBuf, size_t outBufLen){
    return FilePath_ExportEx(path, outBuf, outBufLen, path->depth);
}

int FilePath_ExportDirPath(FilePath *path, char *outBuf, size_t outBufLen){
    return FilePath_ExportEx(path, outBuf, outBufLen, path->depth - 1);
}

int FilePath_Levels(FilePath *path){
    return path->depth;
}

char *FilePath_LevelStr(FilePath *path, int depth){
    if(depth >= 0 && depth < path->depth){
        return path->level[depth].str;
    }
    return NULL;
}

char *FilePath_FileName(FilePath *path){
    return FilePath_LevelStr(path, path->depth - 1);
}

char *FilePath_FileExt(FilePath *path){
    return path->ext;
}

//change a string path into a path object
int FilePath_Convert(FilePath *path, const char *sourcePath, size_t len){
    if(!strncasecmp(sourcePath, DIR_ROOT, strlen(DIR_ROOT))){
        path->absolute = 1;
    }

	//printf("path len: %d\n", len);
	char *temp = calloc(sizeof(char), len + 2);
	if(!temp){
		//printf("error alloc path\n");
		 //BAG_DBG_LibMsg(BUFFER_FAILED_MSG, (int)len + 2, "File Path Conversion");
		return 0;
	}

	strcpy(temp, sourcePath);
	strcat(temp, "\0");

	char delimiters[2] = {DIR_SEPARATOR, '\0'};
	char *entry = strtok(temp, delimiters);

	while(entry){
        size_t len = strlen(entry);
        if(len <= 0) continue;

		FilePath_Add(path, entry);
		entry = strtok(NULL, delimiters);
	}

	free(temp);

    struct stat st;
    lstat(sourcePath, &st);
    path->folder = S_ISDIR(st.st_mode);

	return 1;
}

int FilePath_Copy(FilePath *dest, FilePath *src){
    //clean up the destination
    FilePath_Clean(dest);

    for(int i = 0; i < src->depth; i++){
        if(!FilePath_Add(dest, FilePath_LevelStr(src, i))){
            //BAG_DBG_LibMsg("BAG:Error copying FilePath\n");
            return 0;
        }
    }
    return 1;
}

int FilePath_Cat(FilePath *dest, FilePath *src){
    for(int i = 0; i < src->depth; i++){
        if(!FilePath_Add(dest, FilePath_LevelStr(src, i))){
            //BAG_DBG_LibMsg("BAG:Error copying FilePath\n");
            return 0;
        }
    }
    return 1;
}
