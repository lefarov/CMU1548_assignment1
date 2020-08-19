#include <stdio.h>
#include <algorithm>
#include <getopt.h>
#include <math.h>
#include "CMU418intrin.h"
#include "logger.h"
using namespace std;

#define EXP_MAX 10

Logger CMU418Logger;

void usage(const char* progname);
void initValue(float* values, int* exponents, float* output, float* gold, unsigned int N);
void absSerial(float* values, float* output, int N);
void absVector(float* values, float* output, int N);
void clampedExpSerial(float* values, int* exponents, float* output, int N);
void clampedExpVector(float* values, int* exponents, float* output, int N);
float arraySumSerial(float* values, int N);
float arraySumVector(float* values, int N);
bool verifyResult(float* values, int* exponents, float* output, float* gold, int N);

int main(int argc, char * argv[]) {
  int N = 16;
  bool printLog = false;

  // parse commandline options ////////////////////////////////////////////
  int opt;
  static struct option long_options[] = {
    {"size", 1, 0, 's'},
    {"log", 0, 0, 'l'},
    {"help", 0, 0, '?'},
    {0 ,0, 0, 0}
  };

  while ((opt = getopt_long(argc, argv, "s:l?", long_options, NULL)) != EOF) {

    switch (opt) {
      case 's':
        N = atoi(optarg);
        if (N <= 0) {
          printf("Error: Workload size is set to %d (<0).\n", N);
          return -1;
        }
        break;
      case 'l':
        printLog = true;
        break;
      case '?':
      default:
        usage(argv[0]);
        return 1;
    }
  }


  float* values = new float[N+VECTOR_WIDTH];
  int* exponents = new int[N+VECTOR_WIDTH];
  float* output = new float[N+VECTOR_WIDTH];
  float* gold = new float[N+VECTOR_WIDTH];
  initValue(values, exponents, output, gold, N);

  clampedExpSerial(values, exponents, gold, N);
  clampedExpVector(values, exponents, output, N);

  //absSerial(values, gold, N);
  //absVector(values, output, N);

  printf("\e[1;31mCLAMPED EXPONENT\e[0m (required) \n");
  bool clampedCorrect = verifyResult(values, exponents, output, gold, N);
  if (printLog) CMU418Logger.printLog();
  CMU418Logger.printStats();

  printf("************************ Result Verification *************************\n");
  if (!clampedCorrect) {
    printf("@@@ Failed!!!\n");
  } else {
    printf("Passed!!!\n");
  }

  printf("\n\e[1;31mARRAY SUM\e[0m (bonus) \n");
  if (N % VECTOR_WIDTH == 0) {
    float sumGold = arraySumSerial(values, N);
    float sumOutput = arraySumVector(values, N);
    float epsilon = 0.1;
    bool sumCorrect = abs(sumGold - sumOutput) < epsilon * 2;
    if (!sumCorrect) {
      printf("Expected %f, got %f\n.", sumGold, sumOutput);
      printf("@@@ Failed!!!\n");
    } else {
      printf("Passed!!!\n");
    }
  } else {
    printf("Must have N % VECTOR_WIDTH == 0 for this problem (VECTOR_WIDTH is %d)\n", VECTOR_WIDTH);
  }

  delete[] values;
  delete[] exponents;
  delete[] output;
  delete gold;

  return 0;
}

void usage(const char* progname) {
  printf("Usage: %s [options]\n", progname);
  printf("Program Options:\n");
  printf("  -s  --size <N>     Use workload size N (Default = 16)\n");
  printf("  -l  --log          Print vector unit execution log\n");
  printf("  -?  --help         This message\n");
}

void initValue(float* values, int* exponents, float* output, float* gold, unsigned int N) {

  for (unsigned int i=0; i<N+VECTOR_WIDTH; i++)
  {
    // random input values
    values[i] = -1.f + 4.f * static_cast<float>(rand()) / RAND_MAX;
    exponents[i] = rand() % EXP_MAX;
    output[i] = 0.f;
    gold[i] = 0.f;
  }

}

bool verifyResult(float* values, int* exponents, float* output, float* gold, int N) {
  int incorrect = -1;
  float epsilon = 0.00001;
  for (int i=0; i<N+VECTOR_WIDTH; i++) {
    if ( abs(output[i] - gold[i]) > epsilon ) {
      incorrect = i;
      break;
    }
  }

  if (incorrect != -1) {
    if (incorrect >= N)
      printf("You have written to out of bound value!\n");
    printf("Wrong calculation at value[%d]!\n", incorrect);
    printf("value  = ");
    for (int i=0; i<N; i++) {
      printf("% f ", values[i]);
    } printf("\n");

    printf("exp    = ");
    for (int i=0; i<N; i++) {
      printf("% 9d ", exponents[i]);
    } printf("\n");

    printf("output = ");
    for (int i=0; i<N; i++) {
      printf("% f ", output[i]);
    } printf("\n");

    printf("gold   = ");
    for (int i=0; i<N; i++) {
      printf("% f ", gold[i]);
    } printf("\n");
    return false;
  }
  printf("Results matched with answer!\n");
  return true;
}

void absSerial(float* values, float* output, int N) {
  for (int i=0; i<N; i++) {
    float x = values[i];
    if (x < 0) {
      output[i] = -x;
    } else {
      output[i] = x;
    }
  }
}

// implementation of absolute value using 15418 instrinsics
void absVector(float* values, float* output, int N) {
  __cmu418_vec_float x;
  __cmu418_vec_float result;
  __cmu418_vec_float zero = _cmu418_vset_float(0.f);
  __cmu418_mask maskAll, maskIsNegative, maskIsNotNegative;

//  Note: Take a careful look at this loop indexing.  This example
//  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
//  Why is that the case?
//  -> line 190: maskAll always contains mask for the VECTOR_WIDTH values
//  -> line 197: will always load VECTOR_WIDTH elements: might result in overflow
  for (int i=0; i<N; i+=VECTOR_WIDTH) {

    // All ones
    maskAll = _cmu418_init_ones();

    // All zeros
    maskIsNegative = _cmu418_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _cmu418_vload_float(x, values+i, maskAll);               // x = values[i];

    // Set mask according to predicate
    _cmu418_vlt_float(maskIsNegative, x, zero, maskAll);     // if (x < 0) {

    // Execute instruction using mask ("if" clause)
    _cmu418_vsub_float(result, zero, x, maskIsNegative);      //   output[i] = -x;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotNegative = _cmu418_mask_not(maskIsNegative);     // } else {

    // Execute instruction ("else" clause)
    _cmu418_vload_float(result, values+i, maskIsNotNegative); //   output[i] = x; }

    // Write results back to memory
    _cmu418_vstore_float(output+i, result, maskAll);
  }
}

// accepts and array of values and an array of exponents
//
// For each element, compute values[i]^exponents[i] and clamp value to
// 9.999.  Store result in outputs.
void clampedExpSerial(float* values, int* exponents, float* output, int N) {
  for (int i=0; i<N; i++) {
    float x = values[i];
    int y = exponents[i];
    if (y == 0) {
      output[i] = 1.f;
    } else {
      float result = x;
      int count = y - 1;
      while (count > 0) {
        result *= x;
        count--;
      }
      if (result > 9.999999f) {
        result = 9.999999f;
      }
      output[i] = result;
    }
  }
}

void clampedExpVector(float* values, int* exponents, float* output, int N) {
  // TODO: Implement your vectorized version of clampedExpSerial here
  // Temp variables
  __cmu418_vec_float x;
  __cmu418_vec_int y;
  __cmu418_vec_float result;

  // Temp vectors
  __cmu418_vec_float zero_float = _cmu418_vset_float(0.f);
  __cmu418_vec_float one_float = _cmu418_vset_float(1.f);
  __cmu418_vec_float threshold_float = _cmu418_vset_float(9.999999f);
  __cmu418_vec_int zero_int = _cmu418_vset_int(0);
  __cmu418_vec_int one_int = _cmu418_vset_int(1);

  // Temp masks
  __cmu418_mask maskAll;  // mask for values reading
  __cmu418_mask maskEqualZero;
  __cmu418_mask maskGraterZero;
  __cmu418_mask maskGraterThanThreshold;
  __cmu418_mask maskLessOrEqualThanThreshold;

  
  for (int i=0; i<N; i+=VECTOR_WIDTH) {
    if (i + VECTOR_WIDTH > N) {
      maskAll = _cmu418_init_ones((N-i) % VECTOR_WIDTH);
    } else {
      maskAll = _cmu418_init_ones(VECTOR_WIDTH);
    }
    maskEqualZero = _cmu418_init_ones(0);
    maskGraterZero = _cmu418_init_ones(0);
    maskGraterThanThreshold = _cmu418_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _cmu418_vload_float(x, values+i, maskAll);
    _cmu418_vload_int(y, exponents+i, maskAll);

    // Handle zeros
    // if (y == 0) {
    _cmu418_veq_int(maskEqualZero, y, zero_int, maskAll);
    _cmu418_vmove_float(result, one_float, maskEqualZero);
    // } else {
    // Handle non zero elements
    _cmu418_vgt_int(maskGraterZero, y, zero_int, maskAll);
    _cmu418_vmove_float(result, x, maskGraterZero);

    // Update count mask
    _cmu418_vsub_int(y, y, one_int, maskGraterZero);
    _cmu418_vgt_int(maskGraterZero, y, zero_int, maskAll);
    while (_cmu418_cntbits(maskGraterZero)) {  
      _cmu418_vmult_float(result, result, x, maskGraterZero);
      
      // Update count mask
      _cmu418_vsub_int(y, y, one_int, maskGraterZero);
      _cmu418_vgt_int(maskGraterZero, y, zero_int, maskGraterZero);
    }

    // Clamp value
    _cmu418_vgt_float(maskGraterThanThreshold, result, threshold_float, maskAll);
    _cmu418_vmove_float(result, threshold_float, maskGraterThanThreshold);
    _cmu418_vstore_float(output+i, result, maskAll);
  }
}

float arraySumSerial(float* values, int N) {
  float sum = 0;
  for (int i=0; i<N; i++) {
    sum += values[i];
  }

  return sum;
}

void printArray(float* values, int N) {
  for (int i=0; i<N; i++) {
    printf("%f ", values[i]);
  }
  printf("\n");
}

// Assume N is a power VECTOR_WIDTH == 0
// Assume VECTOR_WIDTH is a power of 2
float arraySumVector(float* values, int N) {
  // TODO: Implement your vectorized version of arraySumSerial here
  int readIndexLeft;
  int readIndexRight;
  int readIndexMiddle = (N-VECTOR_WIDTH)/2;

  __cmu418_vec_float x;
  __cmu418_vec_float xInterleaved;
  __cmu418_mask maskAll = _cmu418_init_ones();

  // Aggregate the values from both sides in the middle of the array.
  // One aggregation operation consits of subsequent application of `hadd` and `interleave`.
  for (unsigned int i=0; i<N; i+=VECTOR_WIDTH) {
    // Aggreagate values from the left side of array by moving with the `VECTOR_WIDTH/2` step.
    // Example for `VECTOR_WIDTH=4`, () denotes aggregated elements:
    // step 0: `[(0 1 2 3) 4 5 ...]`
    // step 1: `[0 1 (2 3 4 5) ...]`
    // etc..
    readIndexLeft = i / 2;
    _cmu418_vload_float(x, values+readIndexLeft, maskAll);
    _cmu418_hadd_float(x, x);
    _cmu418_interleave_float(xInterleaved, x);
    _cmu418_vstore_float(values+readIndexLeft, xInterleaved, maskAll);

    // Aggreagate values from the right side.
    readIndexRight = N - readIndexLeft - VECTOR_WIDTH;
    // At the last iteration `readIndexLeft` is equal to `readIndexRight` and `readIndexMiddle`.
    // Insure that aggregation is not perofromed twice on the same elements.
    if (readIndexLeft != readIndexRight) {
      _cmu418_vload_float(x, values+readIndexRight, maskAll);
      _cmu418_hadd_float(x, x);
      _cmu418_interleave_float(xInterleaved, x);
      _cmu418_vstore_float(values+readIndexRight, xInterleaved, maskAll);
    }
  }

  // Peroform additional aggregations in the middle if `VECTOR_WIDTH>2`.
  for (unsigned int i=0; i<log2(VECTOR_WIDTH) - 1; ++i) {
    _cmu418_vload_float(x, values+readIndexMiddle, maskAll);
    _cmu418_hadd_float(x, x);
    _cmu418_interleave_float(xInterleaved, x);
    _cmu418_vstore_float(values+readIndexLeft, xInterleaved, maskAll);
  }

  return values[(N-VECTOR_WIDTH)/2];
}