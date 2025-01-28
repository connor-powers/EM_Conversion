#include <gtest/gtest.h>
#include "header.h"



//run simple unit test using example from Swarthmore LPSA page on "Analogous Electrical and Mechanical Systems" as ground truth
TEST(ElectricalCircuitOutputTest,TestCase1){
    std::string row_1_groundtruth="[G]-----------[->](5.210)---R(0.667)------I(5.000)------R(0.667)------[G]                                       \n";
    std::string row_2_groundtruth="                                                     |                                                          \n";
    std::string row_3_groundtruth="                                                     ---C(10.000)-----[G]                                       \n";

    MechanicalCircuit mech_circuit("../input_file.txt");
    //let's build the mechanical circuit object from the input file
    ElectricalCircuit elec_circuit(mech_circuit,"Force-Current");
    elec_circuit.draw_electrical_circuit();

    EXPECT_TRUE(elec_circuit.output_string_list_.at(0)==row_1_groundtruth) << "Mismatch detected in first line of output circuit";
    EXPECT_TRUE(elec_circuit.output_string_list_.at(1)==row_2_groundtruth) << "Mismatch detected in second line of output circuit";
    EXPECT_TRUE(elec_circuit.output_string_list_.at(1)==row_3_groundtruth) << "Mismatch detected in third line of output circuit";

}