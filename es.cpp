/**
 * Author: Lukas Dibdak
 * Project: PRL
 * Algorithm: Enumeration sort
 * FIT VUT Brno, 2016/2017
 */

#include <iostream>
#include <mpi.h>
#include <queue>
#include <fstream>

const int MAIN_PROCESS = 0;
const int EMPTY = -1;

const int X = 1;
const int Y = 2;
const int Z = 3;
const int CX = 4;

using namespace std;

int main(int argc, char *argv[]) {
    /** MPI Initialization **/
    MPI_Init(&argc, &argv);

    int numberOfProcesses;
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    numberOfProcesses--;

    int processRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

    /** Initialization of processor's registers **/
    int x = EMPTY, y = EMPTY, z = EMPTY, c = 1;

    /** Reading random numbers from input file **/
    queue<int> numbers;
    if (processRank == MAIN_PROCESS) {
        fstream input;
        input.open("numbers", ios::in);

        bool first = true;
        while (input.good()) {
            int number = input.get();
            if (!input.good()) {
                break;
            }

            numbers.push(number);

            if (first) {
                cout << number;
            } else {
                cout << " " << number;
            }

            first = false;
        }

        cout << endl;
    }

    /** Main program loop **/
    for (int i = 1; i <= 2 * numberOfProcesses; i++) {
        if (processRank == MAIN_PROCESS) {
            if (!numbers.empty()) {
                /** If input sequence is not empty, send number to processes and remove this number from sequence **/
                int numberx = numbers.front();
                int numbery = numbers.front();
                MPI_Send(&numberx, 1, MPI_INT, 1, Y, MPI_COMM_WORLD);
                MPI_Send(&numbery, 1, MPI_INT, i, X, MPI_COMM_WORLD);
                numbers.pop();
            }
        } else {
            int h = (i <= numberOfProcesses) ? 1 : i - numberOfProcesses;

            /** Compare X and Y **/
            if (processRank >= h && x != EMPTY && y != EMPTY && x > y) {
                c++;
            }

            /** Send your Y number to your right processor **/
            if (processRank >= h && processRank < numberOfProcesses && y != EMPTY) {
                int number = y;
                MPI_Send(&number, 1, MPI_INT, processRank + 1, Y, MPI_COMM_WORLD);
            }

            /** Receive Y number from your left processor **/
            if ((i <= numberOfProcesses && processRank >= h && processRank <= i)
                || (i > numberOfProcesses && processRank > h)) {
                int receivedY;
                MPI_Recv(&receivedY, 1, MPI_INT, processRank - 1, Y, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                y = receivedY;
            }

            /** If it is your turn, receive X from main processor **/
            if (i <= numberOfProcesses && processRank == i) {
                int receivedX;
                MPI_Recv(&receivedX, 1, MPI_INT, MAIN_PROCESS, X, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                x = receivedX;
            }

            /** No more input, time to send Z register **/
            if (i > numberOfProcesses && processRank == (i - numberOfProcesses)) {
                int values[] = {x, c};
                for (int i = 1; i <= numberOfProcesses; i++) {
                    MPI_Send(values, 2, MPI_INT, i, CX, MPI_COMM_WORLD);
                }
            }

            /** No more input, time to receive Z register **/
            if (i > numberOfProcesses) {
                int values[2];
                MPI_Recv(values, 2, MPI_INT, (i - numberOfProcesses), CX, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (processRank == values[1]) {
                    z = values[0];
                }
            }
        }
    }

    /** Sending results to main procssors and presenting sorted sequence **/
    if (processRank == MAIN_PROCESS) {
        int lastNumber = EMPTY;
        for (int i = 1; i <= numberOfProcesses; i++) {
            int number;
            MPI_Recv(&number, 1, MPI_INT, i, Z, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (number != EMPTY) {
                cout << number << endl;
            } else if (lastNumber != EMPTY) {
                cout << lastNumber << endl;
            }

            lastNumber = number;
        }
    } else {
        MPI_Send(&z, 1, MPI_INT, MAIN_PROCESS, Z, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}