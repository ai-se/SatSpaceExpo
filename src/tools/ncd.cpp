#include "commons/utility/utility.h"
#include "zlib/zlib.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

// https://panthema.net/2007/0328-ZLibString.html
unsigned
compress_string(const std::string &str,
                int compressionlevel =
                    Z_DEFAULT_COMPRESSION) { // Z_BEST_COMPRESSION Z_BEST_SPEED
  z_stream zs; // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (deflateInit(&zs, compressionlevel) != Z_OK)
    throw(std::runtime_error("deflateInit failed while compressing."));

  zs.next_in = (Bytef *)str.data();
  zs.avail_in = str.size(); // set the z_stream's input

  int ret;
  char outbuffer[32768];
  //   std::string outstring;
  size_t RES = 0;

  // retrieve the compressed bytes blockwise
  do {
    zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = deflate(&zs, Z_FINISH);

    // if (outstring.size() < zs.total_out) {
    //   // append the block to the output string
    //   outstring.append(outbuffer, zs.total_out - outstring.size());
    // }
    if (RES < zs.total_out)
      RES += zs.total_out;
  } while (ret == Z_OK);

  deflateEnd(&zs);

  if (ret != Z_STREAM_END) { // an error occurred that was not EOF
    std::ostringstream oss;
    oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
    throw(std::runtime_error(oss.str()));
  }

  return RES;
}

/**
 * Reference of NCD diversity calc
 * "Test Set Diameter: Quantifying the Diversity of Sets of Test Cases"
 * Robert Feldt et al. ICST 2016
 * See [14] at aforementioned reference
 * NCD = normalized compression distance
 * Use NCD1 as approx here (TODO change to NCD)
 **/
void ncd(std::string file) {
  timer P1;
  std::set<std::string> unique_models;
  std::ifstream infile(file);
  std::string line;
  int total = 0;
  while (std::getline(infile, line)) {
    if (line[0] == '$')
      continue;
    total++;
    unique_models.insert(line);
  }
  std::cout << "INFO " << total << " into uniques#" << unique_models.size()
            << std::endl;
  P1.show_duration("Checking the unique model");

  std::set<std::string> X = std::move(unique_models);

  std::vector<size_t> Cx;
  std::string concatX = "";
  for (auto &x : X) {
    Cx.push_back(compress_string(x));
    concatX += x;
  }

  // P1.show_duration("end of concat");
  size_t CX = compress_string(concatX);
  std::cout << (CX - *std::min_element(Cx.begin(), Cx.end())) << std::endl;
  // P1.show_duration("end of long compressing");
}

int main(int argc, char *argv[]) {
  std::string model = "Benchmarks/polynomial.sk_7_25.cnf";
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
  }

  // the qs.valid file
  std::string file =
      "memo/" + model.substr(model.find_last_of("/") + 1) + ".qs.valid";
  ncd(file);
  return 0;
}