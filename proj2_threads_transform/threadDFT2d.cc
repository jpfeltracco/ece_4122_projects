// Threaded two-dimensional Discrete FFT transform
// Jeremy Feltracco
// ECE4122 Project 2

#include <iostream>
#include <string>
#include <math.h>

#include "Complex.h"
#include "InputImage.h"

// You will likely need global variables indicating how
// many threads there are, and a Complex* that points to the
// 2d image being transformed.

using namespace std;

unsigned N; // Num points in the 1D transform
Complex* image_data;

Complex* horz_weights;
Complex* vert_weights;

int width;
int height;
const int num_threads = 16;

// Function to reverse bits in an unsigned integer
// This assumes there is a global variable N that is the
// number of points in the 1D transform.
unsigned ReverseBits(unsigned v)
{
    //  Provided to students
    unsigned n = N; // Size of array (which is even 2 power k value)
    unsigned r = 0; // Return value

    for (--n; n > 0; n >>= 1)
    {
        r <<= 1;                // Shift return value
        r |= (v & 0x1); // Merge in next bit
        v >>= 1;                // Shift reversal value
    }
    return r;
}

// GRAD Students implement the following 2 functions.
// Undergrads can use the built-in barriers in pthreads.

// Call MyBarrier_Init once in main
// void MyBarrier_Init()// you will likely need some parameters)
// {

// }

// Each thread calls MyBarrier after completing the row-wise DFT
// void MyBarrier() // Again likely need parameters
// {

// }

void ReverseBits(Complex* arr) {
    for (unsigned i = 0; i < N; ++i) {
        unsigned newpos = ReverseBits(i);
        if (i < newpos) {
            Complex c = arr[newpos];
            arr[newpos] = arr[i];
            arr[i] = c;
        }
    }
}

void Transform1D(Complex* h, Complex* w) // removed N argument
{
    // Implement the efficient Danielson-Lanczos DFT here.
    // "h" is an input/output parameter
    // "N" is the size of the array (assume even power of 2)
    ReverseBits(h);

    // Should this reordering be done in place?

    // cout << "reached transform 1d" << endl;
    // 2, 4, 8, ...
    for (unsigned chunk_size = 2; chunk_size <= N; chunk_size *= 2) {
        // cout << "Chunk size: " << chunk_size << endl;
        // loop in chunks of chunk_size
        for (unsigned chunk = 0; chunk != N; chunk += chunk_size) {
            // cout << "\tChunk: " << chunk << endl;
            // perform transform on chunk
            for (unsigned k = 0; k < chunk_size / 2; ++k) {
                // cout << "\t\tk in chunk: " << k << endl;
                // do actual processing
                unsigned even_ind = k + chunk;
                unsigned odd_ind = k + chunk + chunk_size / 2;

                int weight_ind = k * N / chunk_size;

                // calculate FFT(even) + W * FFT(odd)
                Complex even_result = h[even_ind] + w[weight_ind] * h[odd_ind];
                Complex odd_result = h[even_ind] + Complex(-1, 0) * w[weight_ind] * h[odd_ind];

                h[even_ind] = even_result;
                h[odd_ind] = odd_result;
            }
        }
    }

}

void* Transform2DThread(void* v)
{
    // This is the thread starting point.    "v" is the thread number
    // Calculate 1d DFT for assigned rows
    // wait for all to complete
    // Calculate 1d DFT for assigned columns
    // Decrement active count and signal main if all complete
    return 0;
}

void fill_weights(Complex* arr, int N) {
    for (int i = 0; i < N / 2; ++i) {
        arr[i] = Complex(cos(2*M_PI*i/N), -sin(2*M_PI*i/N));
    }
}

void Transform2D(const char* inputFN)
{
    // Do the 2D transform here.
    // Create the helper object for reading the image
    InputImage image(inputFN);
    // Create the global pointer to the image array data
    image_data = image.GetImageData();
    width = image.GetWidth();
    height = image.GetHeight();


    horz_weights = new Complex[width / 2];
    vert_weights = new Complex[height / 2];
    fill_weights(horz_weights, width);
    fill_weights(vert_weights, height);

    // Create 16 threads
    // Wait for all threads complete
    // Write the transformed data

    N = width;
    for (int row = 0; row < height; ++row) {
        Transform1D(image_data + row * width, horz_weights);
    }
    image.SaveImageData("myafter1d.txt", image_data, width, height);
}

int main(int argc, char** argv)
{
    string fn("Tower.txt"); // default file name
    if (argc > 1) fn = string(argv[1]); // if name specified on cmd line
    // MPI initialization here
    Transform2D(fn.c_str()); // Perform the transform.

    // Reverse bits test
    // int len = 8;
    // N = len;
    // Complex* arr = new Complex[len];
    // for (int i = 0; i < len; ++i) {
    //     arr[i] = Complex(i, 0);
    // }

    // ReverseBits(arr);

    // for (int i = 0; i < len; ++i) {
    //     cout << arr[i] << endl;
    // }

}
