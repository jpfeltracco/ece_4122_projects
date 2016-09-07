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

    // Number of CPUs in the whole MPI system
    int num_ranks;
    // This CPU's rank
    int my_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Number of rows to process per cpu
    int row_p_rank = image_height / num_ranks;
    // Number of cols to process per cpu
    int col_p_rank = image_width / num_ranks;

    // Length of array to hold row-wise FT result for one rank
    int row_sect_len = image_width * row_p_rank;
    // Length of array to hold col-wise FT result for one rank
    int col_sect_len = image_height * col_p_rank;
    // Index of the row to start at when doing row-wise FT
    int my_start_row_ind = row_p_rank * my_rank * image_width;
    // Index of the col to start at when doing column-wise FT
    int my_start_col_ind = col_p_rank * my_rank * image_height;

    // Complex* temp_results = new Complex[image_width * row_p_rank];
    // Results of the fourier transform on our section of rows
    Complex* row_results = new Complex[row_sect_len];

    // Index that our row start at in image_data
    //int start_row_offset = my_start_row * image_width;
    for (int local_row = 0; local_row < row_p_rank; ++local_row) {
        // Offset from first row in image_data
        int offset = local_row * image_width;
        // Transform one row at a time, store in row_results
        Transform1D(image_data + my_start_row_ind + offset,
                    image_width,
                    row_results + offset);
    } // At this point, row_results is this rank's section of the row FT

    // Want contiguous array access to elements column-wise because MPIs
    // gather brings sections from multiple ranks into a contiguous array
    // row_results_t has the same length as row_results, just column-wise ordering
    Complex* row_results_t = new Complex[row_sect_len];
    for (int row = 0; row < row_p_rank; ++row) {
        for (int col = 0; col < image_width; ++col) {
            int orig_pos = row * image_width + col;
            int new_pos = col * row_p_rank + row;
            row_results_t[new_pos] = row_results[orig_pos];
        }
    }
    
    // Chunk refers to each block of memory that needs to be sent
    // to the other CPUs
    int chunk_len = row_p_rank * col_p_rank;
    // Contains columns that need to be FT'd, in column-wise order
    // This means data being sent to each CPU is in contiguous memory
    Complex* data_from_cpus = new Complex[col_sect_len];
    for (int rank = 0; rank < num_ranks; ++rank) {
        MPI_Gather(row_results_t + rank * chunk_len,    // send buff
                   chunk_len,                           // send count
                   MPI_CXX_DOUBLE_COMPLEX,              // send type
                   data_from_cpus,                      // recv buff
                   chunk_len,                           // receive count
                   MPI_CXX_DOUBLE_COMPLEX,              // receive type
                   rank,                                // root (rank)
                   MPI_COMM_WORLD);                     // MPI Comm
    } // At this point, data is in the format below
    // num_ranks = 4; image_width = 8, image_height = 8, row_p_rank = 2, col_p_rank = 2
    // |CPU0 Data          |CPU1 Data          |CPU2 Data          |CPU4 Data
    // |col0[0:1] col1[0:1]|col0[2:3] col1[2:3]|col0[4:5] col1[4:5]|col0[6:7] col1[6:7]

    // go back to a normal column-wise order (FT only works if data contiguous)
    // col0[0:7] col1[0:7]
    Complex* col_values = new Complex[col_sect_len];

    for (int rank = 0; rank < num_ranks; ++rank) {
        for (int local_col = 0; local_col < col_p_rank; ++local_col) {
            // Local row refers to row within a data chunk sent by another CPU
            for (int local_row = 0; local_row < row_p_rank; ++local_row) {
                col_values[rank * row_p_rank + local_col * image_height + local_row]
                    = data_from_cpus[rank * chunk_len + local_col * row_p_rank + local_row];
            }
        }
    }

    Complex* col_results = new Complex[col_sect_len];
    for (int local_row = 0; local_row < row_p_rank; ++local_row) {
        // Offset from first row in image_data
        int offset = local_row * image_width;
        Transform1D(col_values + offset,
                    image_height,
                    col_results + offset);
    } // At this point, col_results contains the FT'd columns

    // need for rank 0 to gather all FT'd columns and transpose them

    Complex* transformed_image = NULL;

    if (my_rank == 0) {
        // only allocate memory for rank 0, recv buff is discared
        // in the other ranks
        transformed_image = new Complex[image_width * image_height];
    }

    // The recv args are *only* used when my_rank == 0
    MPI_Gather(col_results,             // send buff
               col_sect_len,            // send count
               MPI_CXX_DOUBLE_COMPLEX,  // send type
               transformed_image,       // recv buff
               col_sect_len,            // recv count
               MPI_CXX_DOUBLE_COMPLEX,  // recv type
               0,                       // rank
               MPI_COMM_WORLD);         // MPI comm

    if (my_rank == 0) {
        ofstream outfile;
        outfile.open("MyAfter2D.txt");
        outfile << image_width << " " << image_height << endl;
        // we have a column-wise array of the data, but we want
        // to output the data row-wise
        for (int row = 0; row < image_height; ++row) {
            for (int col = 0; col < image_width; ++col) {
                outfile << transformed_image[col * image_height + row]
                        << " ";
            }
            outfile << endl;
        }
        outfile.close();
    }




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
    //int my_start_col = num_cols_per_cpu * my_rank;

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



