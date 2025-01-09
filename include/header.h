#include <iostream>
#include <Eigen/Dense>
#include <algorithm>
#include <fstream>

class MassObject
{
    public:
        std::string name_="";
        double mass_={0};
        double moment_of_inertia_={0};

        MassObject(std::string name, double mass, double moment_of_inertia){
            name_=name;
            mass_=mass;
            moment_of_inertia_=moment_of_inertia;
        }

};

class ConnectionObject
{
    public:
        std::string name_="";
        std::string type_="";
        std::string mass_object_1_name_="";
        std::string mass_object_2_name_="";
        double coefficient_={0};

        ConnectionObject(std::string name, std::string type,std::string mass_object_1_name,std::string mass_object_2_name, double coefficient){
            name_=name;
            type_=type;
            mass_object_1_name_=mass_object_1_name;
            mass_object_2_name_=mass_object_2_name;
            coefficient_=coefficient;
        }

};

class MechanicalCircuit
{
    private:
        std::vector<MassObject> mass_list_{};
        std::vector<ConnectionObject> connection_list_{};
        std::vector<std::string> connection_type_list={"self","friction","damping"};

    public:
        MechanicalCircuit(std::string input_file_name){
            std::ifstream input_file(input_file_name);
            std::string line;
            std::string element_type="none";
            std::string element_name;
            double element_mass;
            double element_MOI;
            std::string element_connection_type;
            std::string element_connection_mass_1;
            std::string element_connection_mass_2;
            double element_connection_coefficient;
            //skip empty lines
            while(getline(input_file,line)){
                if (line.compare("")==0){
                    continue;
                }


                std::string new_element_string("@");
                std::string copy_of_line;
                copy_of_line=line;
                copy_of_line.erase(copy_of_line.begin()+1, copy_of_line.end());
                if (copy_of_line.compare(new_element_string)==0){
                    //first add mass or connection element built up over previous input file lines before resetting for next one
                    if (element_type.compare("mass")==0){
                        add_mass_element(element_name,element_mass,element_MOI);
                    }
                    else if (element_type.compare("connection")==0){
                        add_connection(element_name,element_connection_type,element_connection_mass_1,element_connection_mass_2,element_connection_coefficient);
                    }
                    
                    element_name="";
                    element_mass=0;
                    element_MOI=0;
                    element_connection_type="";
                    element_connection_mass_1="";
                    element_connection_mass_2="";
                    element_connection_coefficient=0;

                    if (line=="@mass"){
                        element_type="mass";
                    }
                    else if (line=="@connection"){
                        element_type="connection";
                    }
                }

                //first eliminate spaces in line if they exist
                //haven't gotten this part to work yet
                // size_t space_index=line.find(" ");
                // if (space_index!=std::string::npos){
                //     auto end_position_iterator=std::remove(line.begin(),line.end()," ");
                //     line.erase(end_position_iterator,line.end());
                // }
                //now let's see where the equal sign is
                std::string delim("=");
                size_t index=line.find(delim);
                if (index != std::string::npos){
                    //if it's a valid index
                    std::string key;
                    std::string val;
                    key=line;
                    val=line;


                    key.erase(key.begin()+index,key.end());

                    val.erase(val.begin(),val.begin()+index+1);

                    if (key.compare("name")==0){
                        if ((val.compare("ground")==0) && (element_type.compare("mass")==0)){
                            double infinity=INFINITY;
                            element_name=val;
                            add_mass_element(val, infinity, infinity);
                            continue;
                        }
                        else{
                            element_name=val;
                        }
                        
                    }
                    else if (key.compare("mass")==0){
                        element_mass=stod(val);
                    }
                    else if (key.compare("MOI")==0){
                        element_MOI=stod(val);
                    }
                    else if (key.compare("type")==0){
                        element_connection_type=val;
                    }
                    else if (key.compare("mass_1")==0){
                        element_connection_mass_1=val;
                    }
                    else if (key.compare("mass_2")==0){
                        element_connection_mass_2=val;
                    }
                    else if (key.compare("coefficient")==0){
                        element_connection_coefficient=stod(val);
                    }
                }

            }
        input_file.close();
        //make last element in file, unless it's the ground object which should have already been made if it exists in the input file
        if ((element_type.compare("mass")==0) && (element_name.compare("ground")!=0)){
            add_mass_element(element_name,element_mass,element_MOI);
        }
        else if (element_type.compare("connection")==0){
            add_connection(element_name,element_connection_type,element_connection_mass_1,element_connection_mass_2,element_connection_coefficient);
        }
        build_connection_matrix();
        list_mass_elements();
        list_connection_elements();
        }


    void add_mass_element(std::string name, double mass, double moment_of_inertia){
        MassObject mass_object=MassObject(name,mass,moment_of_inertia);
        mass_list_.insert(mass_list_.end(),mass_object);
    }

    void add_connection(std::string name, std::string type,std::string mass_object_1_name,std::string mass_object_2_name, double coefficient){
        ConnectionObject connection_object=ConnectionObject(name,type,mass_object_1_name,mass_object_2_name,coefficient);
        connection_list_.insert(connection_list_.end(),connection_object);
    }

    void list_mass_elements(){
        for (size_t index=0;index<mass_list_.size();index++){
            MassObject tmp=mass_list_[index];
            std::cout << "Mass element name: " << tmp.name_ << "\n";
            std::cout << "Mass element mass: " << tmp.mass_ << "\n";
            std::cout << "Mass element MOI: " << tmp.moment_of_inertia_ << "\n";
        }
    }
    void list_connection_elements(){
        for (size_t index=0;index<connection_list_.size();index++){
            ConnectionObject tmp=connection_list_[index];
            std::cout << "Connection element name: " << tmp.name_ << "\n";
            std::cout << "Connection element type: " << tmp.type_ << "\n";
            std::cout << "First mass in connection element: " << tmp.mass_object_1_name_ << "\n";
            std::cout << "Second mass in connection element: " << tmp.mass_object_2_name_ << "\n";
            std::cout << "Connection element coefficient: " << tmp.coefficient_ << "\n";
        }
    }

    void build_connection_matrix(){
        int matrix_side_length=mass_list_.size();
        Eigen::MatrixXd coefficient_matrix;
        Eigen::MatrixXd type_matrix;

        coefficient_matrix.resize(matrix_side_length,matrix_side_length);
        type_matrix.resize(matrix_side_length,matrix_side_length);


        std::vector<Eigen::MatrixXd> connection_matrix_={coefficient_matrix,type_matrix}; //2 element vector, each element is a square matrix of side length equal to the number of mass elements

        //think of this as a 2xLxL matrix where the first element at [1,n,m] is the coefficient of the connection between mass elements n and m and the second element at [2,n,m] holds the connection type (e.g., friction)
        //the connection type follows the following convention:
        //0 is self-interaction (coefficient will be 0)
        //1 is friction
        //2 is damping
        for (int mass_1_index{0};mass_1_index<mass_list_.size();mass_1_index++){
            for (int mass_2_index{0};mass_2_index<mass_list_.size();mass_2_index++){
                if (mass_1_index==mass_2_index){
                    (connection_matrix_.at(0))(mass_1_index,mass_2_index)=0.0;
                    (connection_matrix_.at(1))(mass_1_index,mass_2_index)=std::distance(connection_type_list.begin(), std::find(connection_type_list.begin(),connection_type_list.end(),"self"));

                }
                else {
                    std::string mass_1_name=mass_list_.at(mass_1_index).name_;
                    std::string mass_2_name=mass_list_.at(mass_2_index).name_;
                    for (int connection_index{0};connection_index<connection_list_.size();connection_index++){
                        ConnectionObject tmp_connection_obj=connection_list_[connection_index];
                        if ((tmp_connection_obj.mass_object_1_name_==mass_1_name) && (tmp_connection_obj.mass_object_2_name_==mass_2_name)){
                            (connection_matrix_.at(0))(mass_1_index,mass_2_index)=tmp_connection_obj.coefficient_;
                            (connection_matrix_.at(1))(mass_1_index,mass_2_index)=std::distance(connection_type_list.begin(), std::find(connection_type_list.begin(),connection_type_list.end(),tmp_connection_obj.type_));
                        }
                        }
                    }

                }
            }
        }
    };



class ResistorObject
{

    public:
        std::string name_="";
        std::string incoming_connection_name_="";
        std::string outgoing_connection_name_="";
        double resistance_=0.0;

        ResistorObject(std::string name,std::string incoming_connection_name, std::string outgoing_connection_name, double resistance){
            name_=name;
            incoming_connection_name_=incoming_connection_name;
            outgoing_connection_name_=outgoing_connection_name;
            resistance_=resistance;
        }
};

class CapacitorObject
{

    public:
        std::string name_="";
        std::string incoming_connection_name_="";
        std::string outgoing_connection_name_="";
        double capacitance_=0.0;

        CapacitorObject(std::string name,std::string incoming_connection_name, std::string outgoing_connection_name, double capacitance){
            name_=name;
            incoming_connection_name_=incoming_connection_name;
            outgoing_connection_name_=outgoing_connection_name;
            capacitance_=capacitance;
        }
};

class InductorObject
{
    public:
        std::string name_="";
        std::string incoming_connection_name_="";
        std::string outgoing_connection_name_="";
        double inductance_=0.0;

        InductorObject(std::string name,std::string incoming_connection_name, std::string outgoing_connection_name, double inductance){
            name_=name;
            incoming_connection_name_=incoming_connection_name;
            outgoing_connection_name_=outgoing_connection_name;
            inductance_=inductance;
        }
};

class ElectricalCircuit
{
    private:
        std::vector<ResistorObject> resistor_list_{};
        std::vector<CapacitorObject> capacitor_list_{};
        std::vector<InductorObject> inductor_list_{};
        std::vector<std::string> mech_connection_type_list={"self","friction","damping"};

    public:
        ElectricalCircuit(MechanicalCircuit input_mechanical_circuit){

        }
    
    void add_circuit_element(std::string element_type,std::string element_name,std::string incoming_connection_name,std::string outgoing_connection_name,double coefficient){
        if (element_type.compare("resistor")==0){
            ResistorObject new_resistor(element_name,incoming_connection_name,outgoing_connection_name,coefficient);
            resistor_list_.insert(resistor_list_.end(),new_resistor);
        }
        else if (element_type.compare("capacitor")==0){
            CapacitorObject new_capacitor(element_name,incoming_connection_name,outgoing_connection_name,coefficient);
            capacitor_list_.insert(capacitor_list_.end(),new_capacitor);
        }
        else if (element_type.compare("inductor")==0){
            InductorObject new_inductor(element_name,incoming_connection_name,outgoing_connection_name,coefficient);
            inductor_list_.insert(inductor_list_.end(),new_inductor);
        }
    }
};
