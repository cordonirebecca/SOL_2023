//
// Created by rebecca on 07/11/22.
//

#ifndef SOL_AUXILIARYMW_H
#define SOL_AUXILIARYMW_H

extern void signalMask();

char* getPathAssoluto(char* directoryName);

int leggiNFileDaDirectory(int *numFile2,const char *dirName, char** arrayPath, int posizioneArray, short bitConteggio, int *numeroFileLetti);

#endif //SOL_AUXILIARYMW_H
