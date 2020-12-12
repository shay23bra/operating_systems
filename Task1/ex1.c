//
// Created by Shay_bra on 01/12/2020.
//
#include <stdio.h>
#include <sys/fcntl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "string.h"

char **readConfigFile(char *configFile) {
    char *studentsDir = (char *) calloc(1, sizeof(char));
    char *expectedOutPut = (char *) calloc(1, sizeof(char));
    char *testInput = (char *) calloc(1, sizeof(char));

    int studentDirSize = 1;
    int testInputSize = 1;
    int expectedOutPutSize = 1;

    int EnterCounter = 0;
    int configFd = open(configFile, O_RDONLY); // open config fille for reading

    if (configFd == -1) {
        printf("wasnt able to open file\n");
        exit(1);
    }

    char tmp;
    int read1 = read(configFd, &tmp, 1);

    while (read1 > 0) {
        if (tmp == '\n') {
            EnterCounter++;
        } else {
            if (EnterCounter == 0) {
                studentsDir = (char *) realloc(studentsDir, ++studentDirSize);
                studentsDir[studentDirSize - 2] = tmp;
                expectedOutPut[expectedOutPutSize - 1] = '\0';
            } else if (EnterCounter == 1) {
                testInput = (char *) realloc(testInput, ++testInputSize);
                testInput[testInputSize - 2] = tmp;
                expectedOutPut[expectedOutPutSize - 1] = '\0';

            } else if (EnterCounter == 2) {
                expectedOutPut = (char *) realloc(expectedOutPut, ++expectedOutPutSize);
                expectedOutPut[expectedOutPutSize - 2] = tmp;
                expectedOutPut[expectedOutPutSize - 1] = '\0';
            }

        }
        read1 = read(configFd, &tmp, 1);
    }
    char **paths = malloc(3 * sizeof(char *));
    paths[0] = studentsDir;
    paths[1] = testInput;
    paths[2] = expectedOutPut;
    close(configFd);
    return paths;
}

int calcArraySize(char *arr) {
    char *a = arr;
    int size = 0;
    while (*a) {
        a++;
        size++;
    }
    return size;
}

char *getuserProgram(char *studentsPath, struct dirent *de) {
    char *userProgram = (char *) calloc(
            calcArraySize(studentsPath) + (2 * calcArraySize(de->d_name)) + 5,
            sizeof(char));

    strcpy(userProgram, studentsPath);
    strcat(userProgram, "/");
    strcat(userProgram, de->d_name);
    strcat(userProgram, "/");
    return userProgram;
}

char *getOutPath(char *studentsPath, char *userProgram, struct dirent *de) {
    char *programOutPath = calloc(calcArraySize(studentsPath) + calcArraySize(de->d_name) + 11,
                                  sizeof(char));
    strcpy(programOutPath, userProgram);

    strcat(userProgram, de->d_name);
    strcat(userProgram, ".c");
    strcat(programOutPath, "main.out");
    return programOutPath;
}

void compileStudentsPrograms(char *studentsPath, char* testInput, char* expectedOutput) {
    struct dirent *de;
//    int studentsPathSize = calcArraySize(studentsPath);
    DIR *dr = opendir(studentsPath);

    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory");
        exit(1);
    }

    pid_t parent;
    int Pstatus = 0;
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && strcmp(de->d_name, "grades.csv") != 0) {
            //TODO move to different function and dont forget to free memmory
            char *userProgram = getuserProgram(studentsPath, de);
            char studentDir[200];
            strcpy(studentDir, userProgram);
            char *programOutPath = getOutPath(studentsPath, userProgram, de);

            pid_t sonProcess;
            if ((sonProcess = fork()) == 0) {
//                    printf("s1\n");
                execlp("gcc", "gcc", userProgram, "-o", programOutPath, NULL);
            } else {

                char output[200];
                char exeFile[200];
                strcpy(exeFile, studentDir);
                strcpy(output, studentDir);

                strcat(exeFile, "main.out");
                strcat(output, "programOutput");
                int fdInput = open(testInput, O_RDONLY);
                int fdOutPut = open(output, O_WRONLY | O_CREAT);
                int saveoutput = dup(1);
                int saveinput = dup(0);

//                if (wait(&Pstatus) > 0) {

//                    printf("%d\n", getpid());

//                    if ((s2 = fork()) == 0) {
//                        printf("%d\n",getpid());
                    //                        char exeFile[200];
//                    TODO check we managed to open t//he file discriptor exit if not.
                    dup2(fdInput, 0);
                    dup2(fdOutPut, 1);

                    pid_t s2;
                    if ((s2 = fork()) == 0) {
                        execlp(exeFile, exeFile, NULL);
                    }


                    else {
                        dup2(saveoutput, 1);
                        dup2(saveinput, 0);
//                    }

//                        if (wait(&Pstatus) > 0) {
                        pid_t s3;
                        int p3status = 0;
                        pid_t testStatus;

                        if ((s3 = fork()) == 0) {
//                                printf("s3\n");
//                                    printf("Output: %s\n",output);
//                                    printf("Expected: %s\n",expectedOutput);
                            execlp("./comp.out", output, expectedOutput, NULL);
                        }
//                                                        if (wait(&p3status) > 0) {
                        testStatus = waitpid(s3, &p3status, 0);
                        if (WIFSIGNALED(p3status)) {
                            printf("Error\n");
                        }
                        if(testStatus == -1){
                            perror("error calling waitpid()\n");
                            exit(1);
                        }
                        else if (testStatus == s3) {

                            if (WIFEXITED(p3status)) {
                                if (WEXITSTATUS(p3status)) {
                                    printf("p3status: %d\n", p3status);

                                    if ((p3status / 256) == 2) {

                                        printf("%s score: 100\n", de->d_name);

                                    } else if ((p3status / 256) == 1) {
                                        printf("%s score: 0\n", de->d_name);
                                    } else
                                        printf("%s %d\n", de->d_name, p3status);

                                }
                            }
                        }
                    }
//                }

            }
        }

    }

    while ((parent = wait(&Pstatus)) > 0);
    closedir(dr);
}


int main(int argc, char **argv) {
    if (argc != 2) { // incase no 2 files given
        printf("Not Given Config file\n");
        exit(1);
    }


    char *configFile = argv[1];
    char **paths = readConfigFile(configFile);
    char *studentsDir = paths[0];
    char *testInput = paths[1];
    char *expectedOutPut = paths[2];

    pid_t parentPro1;
    int P1status = 0;
    pid_t sonPro1;
    if ((sonPro1 = fork()) == 0) {
        execlp("gcc","gcc" ,"comp.c", "-o", "comp.out", NULL);
    }
    while ((parentPro1 = wait(&P1status)) > 0);

    char* Grades[300];
    strcpy(Grades, studentsDir);
    strcat(Grades, "/grades.csv");

    int fdScores = open(Grades, O_RDWR | O_CREAT );

    compileStudentsPrograms(studentsDir,testInput,expectedOutPut);

    free(studentsDir);
    free(expectedOutPut);
    free(testInput);
    free(paths);
    return 0;
}