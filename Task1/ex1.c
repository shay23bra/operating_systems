//
// Shay Bratslavsky 313363541
// Omer Tati 305168049

#include <stdio.h>
#include <sys/fcntl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "string.h"
#include <sys/wait.h>

char **readConfigFile(char *configFile) {
    char *studentsDir = (char *) calloc(1, sizeof(char));
    char *expectedOutPut = (char *) calloc(1, sizeof(char));
    char *testInput = (char *) calloc(1, sizeof(char));

    int studentDirSize = 1;
    int testInputSize = 1;
    int expectedOutPutSize = 1;

    int EnterCounter = 0;
    int configFd = open(configFile, O_RDONLY); // open config file for reading

    if (configFd == -1) { //open file descriptor returns -1 if couldn't open
        printf("wasn't able to open file\n");
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
    char *userProgram = (char *) calloc(calcArraySize(studentsPath) + (2 * calcArraySize(de->d_name)) + 5,
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
    strcat(programOutPath, "main.exe");
    return programOutPath;
}

void checkStudentsPrograms(char *studentsPath, char *testInput, char *expectedOutput) {
    struct dirent *de;
    int saveoutput = dup(1);
    int saveinput = dup(0);
    DIR *dr = opendir(studentsPath);

    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory");
        exit(1);
    }


    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
            char *userProgram = getuserProgram(studentsPath, de);
            char studentDir[200];
            strcpy(studentDir, userProgram);
            char *programOutPath = getOutPath(studentsPath, userProgram, de);
            int parent_pid1 = getpid();
            pid_t son1;
            son1 = fork();
            if (son1 == 0) {
                execlp("gcc", "gcc", userProgram, "-o", programOutPath, NULL); // COMPILING THE USER PROGRAM
            } else if (son1 < 0) {
                printf("s1 fail\n");
                exit(1);
            } else if (getpid() == parent_pid1) {
                wait(NULL); // ASSUMING USER PROGRAM WILL COMPILE SUCCESSFULLY ( as written in instructions) no check for child status
                char output[200];
                char exeFile[200];
                strcpy(exeFile, studentDir);
                strcpy(output, studentDir);

                strcat(exeFile, "main.exe");
                strcat(output, "programOutput");
                int fdInput = open(testInput, O_RDONLY);
                if (fdInput == -1) { //open file descriptor returns -1 if couldn't open
                    printf("error: can't open file descriptor input\n");
                    exit(1);
                }
                int fdOutPut = open(output, O_WRONLY | O_CREAT, 0777);
                if (fdOutPut == -1) { //open file descriptor returns -1 if couldn't open
                    printf("error: can't open file descriptor output\n");
                    exit(1);
                }

                if (dup2(fdInput, 0) == -1) { //dup2 returns -1 if couldn't duplicate fd
                    printf("dup input fail\n");
                    exit(1);
                }
                if (dup2(fdOutPut, 1) == -1) { //dup2 returns -1 if couldn't duplicate fd
                    printf("dup output fail\n");
                    exit(1);
                }

                int parent_pid2 = getpid();
                pid_t son2;
                son2 = fork();
                if (son2 == 0) {
                    execlp(exeFile, exeFile, NULL); // EXECUTE USER PROGRAM
                } else if (son2 < 0) {
                    printf("son2 fail\n");
                    exit(1);
                } else if (parent_pid2 == getpid()) {
                    wait(NULL); //ASSUMING CHILD SUCCESSES (as said in instructions that the program works)
                    close(fdOutPut);
                    close(fdInput);
                    if (dup2(saveoutput, 1) == -1) {//dup2 returns -1 if couldn't duplicate fd
                        printf("dup screen fail\n");
                        exit(1);
                    }
                    if (dup2(saveinput, 0) == -1) {//dup2 returns -1 if couldn't duplicate fd
                        printf("dup keyboard fail\n");
                        exit(1);
                    }


                    int fdResult = open("./results.csv", O_WRONLY | O_APPEND,
                                        0777); // RESULTS FILE EXISTS SO WE APPEND RESULTS
                    if (fdResult == -1) {
                        printf("error: can't open file descriptor results\n");
                        exit(1);
                    }
                    if (dup2(fdResult, 1) == -1) {//dup2 returns -1 if couldn't duplicate fd
                        printf("dup results fail\n");
                        exit(1);
                    }
                    int parent_pid3 = getpid();
                    pid_t son3;
                    int p3status = 0;
                    son3 = fork();
                    if (son3 == 0) {
                        execlp("./comp.out", "./comp.out", output, expectedOutput,
                               NULL); // COMPARE USER OUTPUT TO EXPECTED OUTPUT
                    } else if (son3 < 0) {
                        printf("son3 fail\n");
                    } else if (parent_pid3 == getpid()) {
                        // WRITE RESULTS TO results.csv
                        waitpid(son3, &p3status, 0);
                        if (WIFSIGNALED(p3status)) {
                            printf("Error\n");
                        }

                        if (WEXITSTATUS(p3status)) {
                            if ((p3status / 256) == 2) {
                                printf("%s : 100\n", de->d_name);

                            } else if ((p3status / 256) == 1) {
                                printf("%s : 0\n", de->d_name);
                            } else
                                printf("%s error\n", de->d_name);

                        }
                        close(fdResult);
                    }
                }

            }
            free(userProgram);
            free(programOutPath);
        }

    }


    closedir(dr);
}

void cleanup(char *studentsPath) {
    struct dirent *de;
    DIR *dr = opendir(studentsPath);

    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory");
        exit(1);
    }

    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
            char *userProgram = getuserProgram(studentsPath, de);
            char studentDir[200];
            strcpy(studentDir, userProgram);
            char *programOutPath = getOutPath(studentsPath, userProgram, de);

            char output[200];
            strcpy(output, studentDir);

            strcat(output, "programOutput");
            pid_t s1;
            s1 = fork();
            if (s1 == 0) {
                execlp("rm", "rm", output, NULL);
            } else {
                wait(NULL); // ASSUMING CHILD SUCCESSES
                free(userProgram);
                free(programOutPath);
            }
        }


    }
}

int main(int argc, char **argv) {
    if (argc != 2) { // incase no 2 files given
        printf("Not Given Config file\n");
        exit(1);
    }


    char *configFile = argv[1];

    // READING CONFIG INFORMATION
    char **paths = readConfigFile(configFile);
    char *studentsDir = paths[0];
    char *testInput = paths[1];
    char *expectedOutPut = paths[2];


    // COMPILING comp.c from part 1
    pid_t sonPro1;
    if ((sonPro1 = fork()) == 0) {
        execlp("gcc", "gcc", "comp.c", "-o", "comp.out", NULL);
    } else {
        wait(NULL); // waiting for comp.c to be compiled and continue to check the students results

        // open empty results csv
        int fdResult = open("./results.csv", O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fdResult == -1) {
            printf("error: can't open file descriptor results\n");
            exit(1);
        }

        // RUNNING STUDENTS PROGRAMS
        checkStudentsPrograms(studentsDir, testInput, expectedOutPut);

        // REMOVING UNNECESSARY FILES AND MEMORY
        cleanup(studentsDir);
        free(studentsDir);
        free(expectedOutPut);
        free(testInput);
        free(paths);
    }
    return 0;
}
