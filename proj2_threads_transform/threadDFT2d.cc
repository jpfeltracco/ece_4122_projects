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

Complex* row_weights;
Complex* col_weights;

pthread_barrier_t barrier;
pthread_mutex_t activeMutex;
pthread_mutex_t coutMutex;
pthread_cond_t  allDoneCondition;

int activeThreads = 0;

int width;
int height;

int rows_per_thread;
int cols_per_thread;

const int NUM_THREADS = 16;

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
            swap(arr[newpos], arr[i]);
        }
    }
}

void Transform1D(Complex* h, Complex* w) // removed N argument
{
    // Implement the efficient Danielson-Lanczos DFT here.
    // "h" is an input/output parameter
    // "N" is the size of the array (assume even power of 2)
    ReverseBits(h);

    Complex neg1(-1, 0);
    // 2, 4, 8, ...
    for (unsigned chunk_size = 2; chunk_size <= N; chunk_size *= 2) {
        // loop in chunks of chunk_size
        for (unsigned chunk = 0; chunk != N; chunk += chunk_size) {
            // perform transform on chunk
            for (unsigned k = 0; k < chunk_size / 2; ++k) {
                // do actual processing
                unsigned even_ind = k + chunk;
                unsigned odd_ind = k + chunk + chunk_size / 2;

                int weight_ind = k * N / chunk_size;

                // calculate FFT(even) + W * FFT(odd)
                Complex even_result = h[even_ind] + w[weight_ind] * h[odd_ind];

                h[odd_ind] = h[even_ind] + neg1 * w[weight_ind] * h[odd_ind];
                h[even_ind] = even_result;
            }
        }
    }
}

void transpose(Complex* mat, int w, int h) {
    // Complicated transpose algorithm that performs in place.
    for (int start = 0; start <= w * h; ++start) {
        int next = start;
        int i = 0;
        do {
            ++i;
            next = (next % h) * w + next / h;
        } while (next > start);

        if (next >= start && i != 1) {
            Complex tmp = mat[start];
            next = start;
            do {
                i = (next % h) * w + next / h;
                mat[next] = (i == start) ? tmp : mat[i];
                next = i;
            } while (next > start);
        }
    }
}

void* Transform2DThread(void* v)
{
    // This is the thread starting point.    "v" is the thread number
    long thread_num = (long) v;
    // Calculate 1d DFT for assigned rows
    N = width;
    for (int row = 0; row < rows_per_thread; ++row) {
        Transform1D(image_data + (thread_num * rows_per_thread + row) * width, row_weights);
    }

    // wait for all to complete
    pthread_barrier_wait(&barrier);

    if (v == 0) {
        // let only thread 0 transpose the array
        transpose(image_data, width, height);
    }

    // wait for thread 0 to finish tranposing
    pthread_barrier_wait(&barrier);

    // Calculate 1d DFT for assigned cols
    N = height;
    for (int col = 0; col < cols_per_thread; ++col) {
        Transform1D(image_data + (thread_num * cols_per_thread + col) * height, col_weights);
    }

    // Wait for all threads to finish.
    pthread_barrier_wait(&barrier);

    if (v == 0) {
        // let only thread 0 tranpose the array again
        transpose(image_data, height, width);
    }

    // Calculate 1d DFT for assigned columns
    // Decrement active count and signal main if all complete
    pthread_mutex_lock(&activeMutex);
    activeThreads--;
    if (activeThreads == 0) {
        pthread_cond_signal(&allDoneCondition);
    }
    pthread_mutex_unlock(&activeMutex);
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


    rows_per_thread = height / NUM_THREADS;
    cols_per_thread = width / NUM_THREADS;

    row_weights = new Complex[width / 2];
    col_weights = new Complex[height / 2];
    fill_weights(row_weights, width);
    fill_weights(col_weights, height);


    pthread_t threads[NUM_THREADS];

    if (pthread_barrier_init(&barrier, NULL, NUM_THREADS)) {
        cout << "Barrier couldn't be initialized" << endl;
    }

    pthread_mutex_init(&activeMutex, 0);
    pthread_mutex_init(&coutMutex, 0);
    pthread_cond_init(&allDoneCondition, 0);

    pthread_mutex_lock(&activeMutex);
    activeThreads = NUM_THREADS;
    // Create 16 threads
    for (long i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], 0, Transform2DThread,  (void*) i);
    }
    // Wait for all threads complete
    pthread_cond_wait(&allDoneCondition, &activeMutex);
    pthread_mutex_unlock(&activeMutex);

    // Write the transformed data
    image.SaveImageData("MyAfter2D.txt", image_data, width, height);

    delete[] row_weights;
    delete[] col_weights;
}

int main(int argc, char** argv)
{
    string fn("Tower.txt"); // default file name
    if (argc > 1) fn = string(argv[1]); // if name specified on cmd line
    // MPI initialization here
    Transform2D(fn.c_str()); // Perform the transform.
}
