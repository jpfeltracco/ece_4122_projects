// Distributed two-dimensional Discrete FFT transform
// Jeremy Feltracco
// ECE4122 Project 1


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <signal.h>
#include <math.h>
#include <mpi.h>

#include "Complex.h"
#include "InputImage.h"

using namespace std;

void Transform1D(Complex* h, int w, Complex* H);

void Print(Complex* h, int rows, int cols) {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            cout << h[row * cols + col] << " ";
        }
        cout << endl;
    }
}

void Transform2D(const char* inputFN) 
{ 
    // Do the 2D transform here.
    // 1) Use the InputImage object to read in the Tower.txt file and
    //    find the width/height of the input image.
    // 2) Use MPI to find how many CPUs in total, and which one
    //    this process is
    // 3) Allocate an array of Complex object of sufficient size to
    //    hold the 2d DFT results (size is width * height)
    // 4) Obtain a pointer to the Complex 1d array of input data
    // 5) Do the individual 1D transforms on the rows assigned to your CPU
    // 6) Send the resultant transformed values to the appropriate
    //    other processors for the next phase.
    // 6a) To send and receive columns, you might need a separate
    //     Complex array of the correct size.
    // 7) Receive messages from other processes to collect your columns
    // 8) When all columns received, do the 1D transforms on the columns
    // 9) Send final answers to CPU 0 (unless you are CPU 0)
    // 9a) If you are CPU 0, collect all values from other processors
    //       and print out with SaveImageData().
    InputImage image(inputFN);  // Create the helper object for reading the image
    int image_width = image.GetWidth();
    int image_height = image.GetHeight();
    Complex* image_data = image.GetImageData();
    // Step (1) in the comments is the line above.
    // Your code here, steps 2-9
    // Step (2)
    int num_cpus = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &num_cpus);
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int num_rows_per_cpu = image_height / num_cpus;
    int my_start_row = num_rows_per_cpu * rank;

    // Step (3)
    Complex* temp_results = new Complex[image_width *  num_rows_per_cpu];

    // Step (4)/(5)
    // Index that our row start at in image_data
    int start_row_offset = my_start_row * image_width;
    for (int local_row = 0; local_row < num_rows_per_cpu; ++local_row) {
        // Offset from first row in image_data
        int offset = local_row * image_width;
        Transform1D(image_data + start_row_offset + offset,
                    image_width,
                    temp_results + offset);
    }

    // Ran transform1D on each row, dumping output in image_results

    int num_cols_per_cpu = num_rows_per_cpu;

    //cout << "PRINTING ROW ORDER" << endl;
    //Print(image_result, num_rows_per_cpu, image_width);

    // We will now transpose the matrix for use in the gather function call
    Complex* temp_t = new Complex[image_width *  num_rows_per_cpu];
    for (int row = 0; row < num_rows_per_cpu; ++row) {
        for (int col = 0; col < image_width; ++col) {
            int orig_pos = row * image_width + col;
            int new_pos = col * num_rows_per_cpu + row;
            temp_t[new_pos] = temp_results[orig_pos];
        }
    }
    //cout << "PRINTING COL ORDER" << endl;
    //Print(image_t, image_width, num_rows_per_cpu);

    Complex* col_result = new Complex[image_height * num_rows_per_cpu];
    for (int col = 0; col < num_rows_per_cpu; ++col) {
        // Each gather fills out a column 
        MPI_Gather(image_t + col * num_cols_per_cpu, // send buff
                   num_rows_per_cpu,                      // send count
                   MPI_CXX_DOUBLE_COMPLEX,                // send type
                   col_result + col * image_height,       // recv buff
                   num_rows_per_cpu,                      // receive count
                   MPI_CXX_DOUBLE_COMPLEX,                // receive type
                   col / num_cols_per_cpu,                // root (rank)
                   MPI_COMM_WORLD);                       // MPI Comm
    }

    
    for (int local_row = 0; local_row < num_rows_per_cpu; ++local_row) {
        // Offset from first row in image_data
        int offset = local_row * image_width;
        Transform1D(col_result + offset,
                    image_width,
                    image_result + offset);
    }

    if (rank == 0) {
        Print(image_result, num_rows_per_cpu, image_width);
    }

    //cout << "RANK: " << rank << endl;
    //// Step (6)
    //if (rank == 0) {
        //for (int i = 0; i < 10; ++i) {
            //cout << "SENT: " << image_result[i] << endl;
        //}
        //MPI_Send(image_result,
                 //10,
                 //MPI_CXX_DOUBLE_COMPLEX,
                 //1,
                 //0,
                 //MPI_COMM_WORLD);
    //} else if (rank == 1) {
        //Complex* other = new Complex[10];
        //MPI_Recv(other,
                 //10,
                 //MPI_C_DOUBLE_COMPLEX,
                 //0,
                 //0,
                 //MPI_COMM_WORLD,
                 //NULL);
        //for (int i = 0; i < 10; ++i) {
            //cout << "RECEIVED: " << other[i] << endl;
        //}
    //}

    //delete[] image_data;
    //delete[] image_result;

    // Another step or something
    //int my_start_col = num_cols_per_cpu * rank;

    //// Place to store columns we need to process
    //Complex* image_recv_cols = new Complex[image_height *  num_cols_per_cpu];
    
    //ofstream outfile;
    //outfile.open("MyAfter2D.txt");
    
    //outfile << image_width << " " << image_height << endl;
    //Complex* h = new Complex[image_width];
    //for (int row = 0; row < image_height; row++) {
        //Transform1D(image_data + row * image_width, image_width, h);
        //for (int col = 0; col < image_width; ++col) {
            //outfile << h[col] << " ";
        //}
        //outfile << endl;
    //}

    //outfile.close();
    //delete[] h;
}

void Transform1D(Complex* h, int w, Complex* H)
{
    // Implement a simple 1-d DFT using the double summation equation
    // given in the assignment handout.  h is the time-domain input
    // data, w is the width (N), and H is the output array.
    //cout << "1D Transform width: " << w;
    for (int i = 0; i < w; ++i) {
        Complex result;
        for (int k = 0; k < w; ++k) {
            result = result + h[k] * cos(2*M_PI*i*k / w)
                - Complex(0, 1) * h[k] * sin(2*M_PI*i*k / w);
        }
        //cout << "Transform result: " << result.imag << endl;
        H[i] = result;
    }
}

int main(int argc, char** argv)
{
    string fn("Tower.txt"); // default file name
    if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
    // MPI initialization here
    MPI_Init(&argc, &argv);
    Transform2D(fn.c_str()); // Perform the transform.
    // Finalize MPI here
    MPI_Finalize();
}  



