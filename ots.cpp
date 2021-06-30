/**
 * PRL project 2: odd-even transposition sort
 * 
 * @filename: ots.cpp
 * @author: Peter Lukac
 * @login: xlukac11
 * 
 * */

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#define TAG 0

int load_and_send_numbers(int proc_count, std::string filename="numbers"){

    std::ifstream number_file(filename, std::ifstream::binary);
    if (number_file){
        int idx = 0;
        while (true){
            int number = number_file.get();
            if (number_file.eof())
                break;
            if (idx > 0)
                std::cout << " ";
            std::cout << number;
            // check available procesess for the numbers
            if (idx >=  proc_count)
                return 0;
            // send number to the process idx and increment idx
            MPI_Send(&number, 1, MPI_INT, idx++, TAG, MPI_COMM_WORLD);
        }
        std::cout << std::endl;
        number_file.close();
        return idx;
    }
    else{
        std::cerr << "Couldn't open the file: " << filename << std::endl;
        return 0;
    }
    
}


int main(int argc, char** argv){
    MPI_Status mpi_status; 
    int proc_idx;
    int proc_count;
    int proc_number;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_count);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_idx);

    // main proces will load file with numbers and send in to everyone
    if (proc_idx == 0){
        int load_count = load_and_send_numbers(proc_count);
        if(load_count != proc_count){
            std::cerr << "Missmatch between proc count(" << proc_count << ") and file length" << std::endl;
            MPI_Finalize();
            return 1;
        }
    }

    // everyone recives it's number
    MPI_Recv(&proc_number, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &mpi_status);

    // main loop governing the odd-even transposition sort
    //auto start_time = std::chrono::steady_clock::now();
    for (int i = 0; i < proc_count; i++){
        // process is sending in this part, odd_proc if mod == 0; even_proc if mod == 1; because indexed from zero for both iteration and proc idx
        if (i%2 == proc_idx%2){
            // don't send if you are the last process
            if ( proc_idx + 1 < proc_count ){
                MPI_Send(&proc_number, 1, MPI_INT, proc_idx+1, TAG, MPI_COMM_WORLD);
                MPI_Recv(&proc_number, 1, MPI_INT, proc_idx+1, TAG, MPI_COMM_WORLD, &mpi_status);
            }
        }
        // process is reciving
        else{
            // don't wait for number if you are the first process
            if ( proc_idx != 0 ){
                int recived_number;
                MPI_Recv(&recived_number, 1, MPI_INT, proc_idx-1, TAG, MPI_COMM_WORLD, &mpi_status);
                // if number recived is greater than the current number, send back lower number and store higher number
                if (recived_number > proc_number){
                    MPI_Send(&proc_number, 1, MPI_INT, proc_idx-1, TAG, MPI_COMM_WORLD);
                    proc_number = recived_number;
                }
                // else just send back recived number
                else
                    MPI_Send(&recived_number, 1, MPI_INT, proc_idx-1, TAG, MPI_COMM_WORLD);
            }
        }
    }
    //auto time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_time).count();
    // main proces will colect values from the other processes
    if (proc_idx == 0){
        // print my number first
        std::cout << proc_number << std::endl;
        //std::cerr << time << std::endl;
        int number = 0;
        for (int i = 1; i < proc_count; i++){
            MPI_Recv(&number, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, &mpi_status);
            std::cout << number << std::endl;
	        //MPI_Recv(&time, 8, MPI_INT, i, TAG, MPI_COMM_WORLD, &mpi_status);
	        //std::cerr << time << std::endl;
        }
    }
    // other processes will send their number to main process
    else {
        MPI_Send(&proc_number, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
	    //MPI_Send(&time, 8, MPI_INT, 0, TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
