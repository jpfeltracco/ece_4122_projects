// ECE4122/6122 RSA Encryption/Decryption assignment
// Fall Semester 2015

#include <iostream>
#include "RSA_Algorithm.h"

using namespace std;

// debug
int good = 0;
int bad  =0;

const int minSz = 32;
const int maxSz = 1024; // including
const int numKeysPerSz = 100;
const int numMessagesPerKey = 100;

int main()
{
  // Instantiate the one and only RSA_Algorithm object
  RSA_Algorithm RSA;


  // RSA.GenerateRandomKeyPair(1000);
  // Loop from sz = 32 to 1024 inclusive
  int sz = minSz;
  while (sz <= maxSz) {
      // cout << "sz: " << sz << endl;
      for (int i = 0; i < numKeysPerSz; ++i) {
          // cout << "  key: " << i << endl;
          RSA.GenerateRandomKeyPair(sz);
          int num_n_bits = mpz_sizeinbase(RSA.n.get_mpz_t(), 2);
          RSA.PrintNDE();
          for (int j = 0; j < numMessagesPerKey; ++j) {
              mpz_class m = RSA.rng.get_z_bits(num_n_bits - 1);
              RSA.PrintM(m);
              mpz_class c = RSA.Encrypt(m);
              RSA.PrintC(c);
              mpz_class rm = RSA.Decrypt(c);

              // RSA.PrintM(m);
              // RSA.PrintM(rm);

              // if (m == rm) {
              //     cout << "correct" << endl;
              // } else {
              //     cout << "incorrect" << endl;
              // }
              if (m != rm) {
                  cout << "incorrect" << endl;
              }

          }
      }
      sz *= 2;
  }
}
  
