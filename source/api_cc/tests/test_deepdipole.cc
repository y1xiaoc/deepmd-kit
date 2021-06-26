#include <gtest/gtest.h>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <vector>
#include "DeepTensor.h"
#include "neighbor_list.h"
#include "test_utils.h"

#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  

class TestInferDeepDipole : public ::testing::Test
{  
protected:  
  std::vector<double> coord = {
    12.83, 2.56, 2.18,
    12.09, 2.87, 2.74,
    00.25, 3.32, 1.68,
    3.36, 3.00, 1.81,
    3.51, 2.51, 2.60,
    4.27, 3.22, 1.56 
  };
  std::vector<int> atype = {
    0, 1, 1, 0, 1, 1
  };
  std::vector<double> box = {
    13., 0., 0., 0., 13., 0., 0., 0., 13.
  };
  std::vector<double> expected_d = {
    -9.274180565967479195e-01,2.698028341272042496e+00,2.521268387140979117e-01,2.927260638453461628e+00,-8.571926301526779923e-01,1.667785136187720063e+00
  };
  int natoms;

  deepmd::DeepTensor dp;

  void SetUp() override {
    std::string file_name = "../../tests/infer/deepdipole.pbtxt";
    int fd = open(file_name.c_str(), O_RDONLY);
    tensorflow::protobuf::io::ZeroCopyInputStream* input = new tensorflow::protobuf::io::FileInputStream(fd);
    tensorflow::GraphDef graph_def;
    tensorflow::protobuf::TextFormat::Parse(input, &graph_def);
    delete input;
    std::fstream output("deepdipole.pb", std::ios::out | std::ios::trunc | std::ios::binary);
    graph_def.SerializeToOstream(&output);
    // check the string by the following commands
    // string txt;
    // tensorflow::protobuf::TextFormat::PrintToString(graph_def, &txt);

    dp.init("deepdipole.pb");

    natoms = expected_d.size();
  };

  void TearDown() override {
    remove( "deepdipole.pb" ) ;
  };
};


TEST_F(TestInferDeepDipole, cpu_build_nlist)
{
  EXPECT_EQ(dp.cutoff(), 4.);
  EXPECT_EQ(dp.numb_types(), 2);
  EXPECT_EQ(dp.output_dim(), 3);
  std::vector<int> sel_types = dp.sel_types();
  EXPECT_EQ(sel_types.size(), 1);
  EXPECT_EQ(sel_types[0], 0);

  std::vector<double> value;
  dp.compute(value, coord, atype, box);

  EXPECT_EQ(value.size(), expected_d.size());
  for(int ii = 0; ii < expected_d.size(); ++ii){
    EXPECT_LT(fabs(value[ii] - expected_d[ii]), 1e-10);
  }
}

TEST_F(TestInferDeepDipole, cpu_lmp_nlist)
{
  float rc = dp.cutoff();
  int nloc = coord.size() / 3;  
  std::vector<double> coord_cpy;
  std::vector<int> atype_cpy, mapping;  
  std::vector<int> ilist(nloc), numneigh(nloc);
  std::vector<int*> firstneigh(nloc);
  std::vector<std::vector<int > > nlist_data;
  deepmd::InputNlist inlist(nloc, &ilist[0], &numneigh[0], &firstneigh[0]);
  _build_nlist(nlist_data, coord_cpy, atype_cpy, mapping,
	       coord, atype, box, rc);
  int nall = coord_cpy.size() / 3;
  convert_nlist(inlist, nlist_data);  

  std::vector<double> value;
  dp.compute(value, coord_cpy, atype_cpy, box, nall-nloc, inlist);

  EXPECT_EQ(value.size(), expected_d.size());
  for(int ii = 0; ii < expected_d.size(); ++ii){
    EXPECT_LT(fabs(value[ii] - expected_d[ii]), 1e-10);
  }
}


class TestInferDeepDipoleFake : public ::testing::Test
{  
protected:  
  std::vector<double> coord = {
    12.83, 2.56, 2.18,
    12.09, 2.87, 2.74,
    00.25, 3.32, 1.68,
    3.36, 3.00, 1.81,
    3.51, 2.51, 2.60,
    4.27, 3.22, 1.56 
  };
  std::vector<int> atype = {
    0, 1, 1, 0, 1, 1
  };
  std::vector<double> box = {
    13., 0., 0., 0., 13., 0., 0., 0., 13.
  };
  std::vector<double> expected_d = {
    -3.186217894664857830e-01, 1.082220317383403296e+00, 5.646623185237639730e-02, 7.426508038929955369e-01, -3.115996324658170114e-01, -5.619108089573777720e-01, -4.181578166874897473e-01, -7.579762930974662805e-01, 4.980618433125854616e-01, 1.059635561913792712e+00, -2.641989315855929332e-01, 5.307984468104405273e-01, -1.484512535335152095e-01, 4.978588497891502374e-01, -8.022467807199461509e-01, -9.165936539882671985e-01, -2.238112120606238209e-01, 2.553133145814526217e-01
  };
  int natoms;

  deepmd::DeepTensor dp;

  void SetUp() override {
    std::string file_name = "../../tests/infer/deepdipole_fake.pbtxt";
    int fd = open(file_name.c_str(), O_RDONLY);
    tensorflow::protobuf::io::ZeroCopyInputStream* input = new tensorflow::protobuf::io::FileInputStream(fd);
    tensorflow::GraphDef graph_def;
    tensorflow::protobuf::TextFormat::Parse(input, &graph_def);
    delete input;
    std::fstream output("deepdipole_fake.pb", std::ios::out | std::ios::trunc | std::ios::binary);
    graph_def.SerializeToOstream(&output);
    // check the string by the following commands
    // string txt;
    // tensorflow::protobuf::TextFormat::PrintToString(graph_def, &txt);

    dp.init("deepdipole_fake.pb");

    natoms = expected_d.size();
  };

  void TearDown() override {
    remove( "deepdipole_fake.pb" ) ;
  };
};


TEST_F(TestInferDeepDipoleFake, cpu_build_nlist)
{
  EXPECT_EQ(dp.cutoff(), 2.);
  EXPECT_EQ(dp.numb_types(), 2);
  EXPECT_EQ(dp.output_dim(), 3);
  std::vector<int> sel_types = dp.sel_types();
  EXPECT_EQ(sel_types.size(), 1);
  EXPECT_EQ(sel_types[0], 0);
  EXPECT_EQ(sel_types[1], 1);

  std::vector<double> value;
  dp.compute(value, coord, atype, box);

  EXPECT_EQ(value.size(), expected_d.size());
  for(int ii = 0; ii < expected_d.size(); ++ii){
    EXPECT_LT(fabs(value[ii] - expected_d[ii]), 1e-10);
  }
}

TEST_F(TestInferDeepDipoleFake, cpu_lmp_nlist)
{
  float rc = dp.cutoff();
  int nloc = coord.size() / 3;  
  std::vector<double> coord_cpy;
  std::vector<int> atype_cpy, mapping;  
  std::vector<int> ilist(nloc), numneigh(nloc);
  std::vector<int*> firstneigh(nloc);
  std::vector<std::vector<int > > nlist_data;
  deepmd::InputNlist inlist(nloc, &ilist[0], &numneigh[0], &firstneigh[0]);
  _build_nlist(nlist_data, coord_cpy, atype_cpy, mapping,
	       coord, atype, box, rc);
  int nall = coord_cpy.size() / 3;
  convert_nlist(inlist, nlist_data);  

  std::vector<double> value;
  dp.compute(value, coord_cpy, atype_cpy, box, nall-nloc, inlist);

  EXPECT_EQ(value.size(), expected_d.size());
  for(int ii = 0; ii < expected_d.size(); ++ii){
    EXPECT_LT(fabs(value[ii] - expected_d[ii]), 1e-10);
  }
}

