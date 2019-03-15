#include "commons/utility/utility.h"
#include "zlib/zlib.h"
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>

// https://panthema.net/2007/0328-ZLibString.html
size_t
compress_string(const std::string &str,
                int compressionlevel =
                    Z_DEFAULT_COMPRESSION) { // Z_BEST_COMPRESSION Z_BEST_SPEED
                                             // Z_DEFAULT_COMPRESSION
  z_stream zs; // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (deflateInit(&zs, compressionlevel) != Z_OK)
    throw(std::runtime_error("deflateInit failed while compressing."));

  zs.next_in = (Bytef *)str.data();
  zs.avail_in = str.size(); // set the z_stream's input

  int ret;
  char outbuffer[32768];
  std::string outstring;
  // size_t RES = 0;

  // retrieve the compressed bytes blockwise
  do {
    zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = deflate(&zs, Z_FINISH);

    if (outstring.size() < zs.total_out) {
      // append the block to the output string
      outstring.append(outbuffer, zs.total_out - outstring.size());
    }
    // if (RES < zs.total_out)
    //   RES += zs.total_out;
  } while (ret == Z_OK);

  deflateEnd(&zs);

  if (ret != Z_STREAM_END) { // an error occurred that was not EOF
    std::ostringstream oss;
    oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
    throw(std::runtime_error(oss.str()));
  }

  return outstring.length();
}

/**
 * Reference of NCD diversity calc
 * "Test Set Diameter: Quantifying the Diversity of Sets of Test Cases"
 * Robert Feldt et al. ICST 2016
 * See [14] at aforementioned reference
 * NCD = normalized compression distance
 * Use NCD1 as approx here (TODO change to NCD)
 * writing out the results as ...<<<NCD1<<<...
 **/
void ncd(std::string file, double max_time, std::ofstream &ofs) {
  timer P1;
  std::ifstream infile(file);
  std::string line;

  size_t minCx = UINT_MAX; // max unsigned int
  size_t maxCxx = 0;
  std::string concatX = "";

  std::getline(infile, line);
  size_t unit_l = line.length();
  int s = 0;
  int e = 0;
  double lst_record_time = -1000; // make sure the first start can be recorded

  while (std::getline(infile, line)) {
    if (line[0] != '#') {
      concatX += line;
      minCx = std::min(minCx, compress_string(line));
      e++;
      continue;
    }
    // otherwise, do recording
    std::istringstream iss(line);
    std::string ts;
    double curr_time;
    iss >> ts;
    iss >> curr_time;
    // skip to record if two milestones are too closed
    if (curr_time - lst_record_time < 5 * 60) // 5 minutes
      continue;

    lst_record_time = curr_time;
    ofs << "# " << curr_time; // record time
    iss >> ts;
    ofs << " " << ts; // record valid sample
    iss >> ts;
    iss >> ts;
    ofs << " " << ts << " "; // reocrd toal sample

    size_t CX = compress_string(concatX);
    int sample_counter = 0;
    P1.startnow();
    do {
      int i = (rand() % (e - s)) + s;
      std::string tmp =
          concatX.substr(0, i * unit_l) + concatX.substr((i + 1) * unit_l);
      maxCxx = std::max(maxCxx, compress_string(tmp));
    } while (sample_counter++ < (e - s) &&
             P1.duration() < max_time / 30); // max sample time in secs

    ofs << static_cast<double>(CX - minCx) / static_cast<double>(maxCxx)
        << std::endl;
    s = e + 1;
  } // end of reading
}

/**
 * Get the diversity info
 * run as "bin/ncd -i 10 -t 180"
 * -i MODEL_ID
 * -t MAX_RUNNING_TIME_IN_SECS
 * -e EXTENSION INSIDE THE MEMO FILE
 **/
int main(int argc, char *argv[]) {
  std::string model = "Benchmarks/polynomial.sk_7_25.cnf";
  double max_time = 180;
  std::string extension = "qs.valid";

  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "L"))
      model = "Benchmarks/enqueueSeqSK.sk_10_42.cnf";
    if (!strcmp(argv[i], "test"))
      model = "Benchmarks/test.cnf";
    if (!strcmp(argv[i], "M"))
      model = "Benchmarks/ActivityService.sk_11_27.cnf";
    if (!strcmp(argv[i], "-m"))
      model = argv[i + 1];

    if (!strcmp(argv[i], "-i"))
      model = benchmark_models[atoi(argv[i + 1])];
    if (!strcmp(argv[i], "-t"))
      max_time = double(atoi(argv[i + 1]));
    if (!strcmp(argv[i], "-e"))
      extension = argv[i + 1];
  }

  std::string file =
      "memo/" + model.substr(model.find_last_of("/") + 1) + "." + extension;
  std::cout << "INFO : Checking diversity of file " << file << std::endl;

  std::ofstream ofs;

  ofs.open(file.substr(0, file.find_last_of(".") + 1) + "report",
           std::ofstream::out | std::ofstream::app);
  ofs << "*****" << std::endl;
  ncd(file, max_time, ofs);
  ofs.close();
  return 0;
}