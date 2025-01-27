#include "header.h"


int main(){

    MechanicalCircuit mech_circuit("../input_file.txt");
    //let's build the mechanical circuit object from the input file
    ElectricalCircuit elec_circuit(mech_circuit,"Force-Current");
    elec_circuit.draw_electrical_circuit();


    return 0;
}