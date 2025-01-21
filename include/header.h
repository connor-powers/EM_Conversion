#include <iostream>
#include <Eigen/Dense>
#include <algorithm>
#include <fstream>
#include <unordered_map>

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
        std::string left_element_name_="";
        std::string right_element_name_="";
        double coefficient_={0};

        ConnectionObject(std::string name, std::string type,std::string left_element_name,std::string right_element_name, double coefficient){
            name_=name;
            type_=type;
            left_element_name_=left_element_name;
            right_element_name_=right_element_name;
            coefficient_=coefficient;
        }

};

class ForceObject
{
    public:
        std::string name_="";
        std::string object_name_="";
        double force_val_={0};

        ForceObject(std::string name,std::string object_name, double force_val){
            name_=name;
            object_name_=object_name;
            force_val_=force_val;
        }

};

class MechanicalCircuit
{
    public:
        std::vector<MassObject> mass_list_{};
        std::vector<ConnectionObject> connection_list_{};
        std::vector<ForceObject> force_list_{};
        std::vector<std::string> connection_type_list={"self","damper","spring"};

        MechanicalCircuit(std::string input_file_name){
            std::ifstream input_file(input_file_name);
            std::string line;
            std::string element_type="none";
            std::string element_name;
            double element_mass;
            double element_MOI;
            std::string element_connection_type;
            std::string left_element_name;
            std::string right_element_name;
            std::string object_name;
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
                    if ((element_type.compare("mass")==0)&&(element_name.compare("ground")!=0)){
                        add_mass_element(element_name,element_mass,element_MOI);
                    }
                    else if (element_type.compare("connection")==0){
                        add_connection(element_name,element_connection_type,left_element_name,right_element_name,element_connection_coefficient);
                    }
                    else if (element_type.compare("force")==0){
                        add_force(element_name,object_name,element_connection_coefficient);
                    }
                    
                    element_name="";
                    element_mass=0;
                    element_MOI=0;
                    element_connection_type="";
                    left_element_name="";
                    right_element_name="";
                    object_name="";
                    element_connection_coefficient=0;

                    if (line=="@mass"){
                        element_type="mass";
                    }
                    else if (line=="@connection"){
                        element_type="connection";
                    }
                    else if (line=="@force"){
                        element_type="force";
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
                    else if (key.compare("left_element")==0){
                        left_element_name=val;
                    }
                    else if (key.compare("right_element")==0){
                        right_element_name=val;
                    }
                    else if (key.compare("object")==0){
                        object_name=val;
                    }
                    else if (key.compare("coefficient")==0){
                        element_connection_coefficient=stod(val);
                    }
                    else if (key.compare("force_val")==0){
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
            add_connection(element_name,element_connection_type,left_element_name,right_element_name,element_connection_coefficient);
        }
        else if (element_type.compare("force")==0){
            add_force(element_name,object_name,element_connection_coefficient);
        }
        build_connection_matrix();
        list_mass_elements();
        list_connection_elements();
        }

    void add_mass_element(std::string name, double mass, double moment_of_inertia){
        std::cout << "adding mass object with name, mass and MOI of " << name << ", " <<mass <<", "<< moment_of_inertia << "\n";
        MassObject mass_object=MassObject(name,mass,moment_of_inertia);
        mass_list_.insert(mass_list_.end(),mass_object);
    }

    void add_connection(std::string name, std::string type,std::string left_element_name,std::string right_element_name, double coefficient){
        ConnectionObject connection_object=ConnectionObject(name,type,left_element_name,right_element_name,coefficient);
        connection_list_.insert(connection_list_.end(),connection_object);
    }

    void add_force(std::string name,std::string mass_object_1_name,double force_val){
        ForceObject force_object=ForceObject(name,mass_object_1_name,force_val);
        force_list_.insert(force_list_.end(),force_object);
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
            std::cout << "Object on left of connection: " << tmp.left_element_name_ << "\n";
            std::cout << "Object on right of connection: " << tmp.right_element_name_ << "\n";
            std::cout << "Connection element coefficient: " << tmp.coefficient_ << "\n";
        }
    }
    void list_forces(){
        for (size_t index=0;index<force_list_.size();index++){
            ForceObject tmp=force_list_[index];
            std::cout << "Force name: " << tmp.name_ << "\n";
            std::cout << "Mass element acted on by force: " << tmp.object_name_ << "\n";
            std::cout << "Force value: " << tmp.force_val_ << "\n";
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
        //1 is damper
        //2 is spring
        for (int left_obj_index{0};left_obj_index<mass_list_.size();left_obj_index++){
            for (int right_obj_index{0};right_obj_index<mass_list_.size();right_obj_index++){
                if (left_obj_index==right_obj_index){
                    (connection_matrix_.at(0))(left_obj_index,right_obj_index)=0.0;
                    (connection_matrix_.at(1))(left_obj_index,right_obj_index)=std::distance(connection_type_list.begin(), std::find(connection_type_list.begin(),connection_type_list.end(),"self"));

                }
                else {
                    std::string left_obj_name=mass_list_.at(left_obj_index).name_;
                    std::string right_obj_name=mass_list_.at(right_obj_index).name_;
                    for (int connection_index{0};connection_index<connection_list_.size();connection_index++){
                        ConnectionObject tmp_connection_obj=connection_list_[connection_index];
                        if ((tmp_connection_obj.left_element_name_==left_obj_name) && (tmp_connection_obj.right_element_name_==right_obj_name)){
                            (connection_matrix_.at(0))(left_obj_index,right_obj_index)=tmp_connection_obj.coefficient_;
                            (connection_matrix_.at(1))(left_obj_index,right_obj_index)=std::distance(connection_type_list.begin(), std::find(connection_type_list.begin(),connection_type_list.end(),tmp_connection_obj.type_));
                        }
                        }
                    }

                }
            }
        }
    };




class ElectricalCircuitElement
{
    public:
        std::string name_="";
        std::string type_="";
        std::vector<ElectricalCircuitElement> left_circuit_elements; 
        std::vector<ElectricalCircuitElement> right_circuit_elements;
        double coefficient_=0.0;

        ElectricalCircuitElement(std::string name, std::string type,double coefficient){
            name_=name;
            type_=type;
            coefficient_=coefficient;
        }

        void add_left_element(ElectricalCircuitElement input_element){
            std::cout << "adding element to left of circuit element named " << name_ << "\n";
            std::cout << "pre-push size: " << left_circuit_elements.size() << "\n";

            left_circuit_elements.push_back(input_element);
            std::cout << "post-push size: " << left_circuit_elements.size() << "\n";
        }
        void add_right_element(ElectricalCircuitElement input_element){
            std::cout << "adding element to right of circuit element named " << name_ << "\n";
            std::cout << "pre-push size: " << right_circuit_elements.size() << "\n";

            right_circuit_elements.push_back(input_element);
            std::cout << "post-push size: " << right_circuit_elements.size() << "\n";
        }
        bool operator==(const ElectricalCircuitElement& other_element) const {
            return (name_==other_element.name_);
        }
};

class ElectricalCircuit
{
    private:
        std::vector<ElectricalCircuitElement> circuit_element_list_{};
        // std::vector<WireObject> wire_list_{}; //now not sure this will be necessary
        std::unordered_map<std::string,ElectricalCircuitElement> circuit_element_map_; //want to map names to specific elements

    public:
        ElectricalCircuit(MechanicalCircuit input_mechanical_circuit,std::string analogy){
            if (analogy.compare("Force-Current")==0){
                force_current_conversion(input_mechanical_circuit);
            }
        }
    
    void add_circuit_element(std::string element_name,std::string element_type,double coefficient){
        ElectricalCircuitElement new_element(element_name,element_type,coefficient);
        circuit_element_list_.push_back(new_element);
        circuit_element_map_.emplace(element_name,circuit_element_list_.at(circuit_element_list_.size()-1));
    }
    



    void force_current_conversion(MechanicalCircuit input_mechanical_circuit){
        //this version is using the force-current analogy
        //first, convert masses to grounded capacitors

        for (size_t mass_index=0; mass_index<input_mechanical_circuit.mass_list_.size();mass_index++){
            MassObject mass_element=input_mechanical_circuit.mass_list_[mass_index];
            add_circuit_element(mass_element.name_+"_ground","ground",0);
            add_circuit_element(mass_element.name_,"capacitor",mass_element.mass_);
        }

        for (size_t connection_index=0; connection_index<input_mechanical_circuit.connection_list_.size();connection_index++){
            //next, convert dampers to resistors and add the elements they are connected to (if they don't exist yet)
            ConnectionObject connection_element=input_mechanical_circuit.connection_list_[connection_index];
            if (connection_element.type_.compare("damper")==0){
                add_circuit_element(connection_element.name_,"resistor",1/connection_element.coefficient_);
            }
            //next convert springs to inductors
            else if (connection_element.type_.compare("spring")==0){
                add_circuit_element(connection_element.name_,"capacitor",1/connection_element.coefficient_);
            }
        }
        //now convert forces to current sources and connect it to the corresponding circuit element and a ground (assuming ground is to the left of forces for now)
        for (size_t force_index=0; force_index<input_mechanical_circuit.force_list_.size();force_index++){
            ForceObject force_element=input_mechanical_circuit.force_list_[force_index];
            add_circuit_element(force_element.name_+"_ground","ground",0);
            add_circuit_element(force_element.name_,"current_source",force_element.force_val_);
        }

        //now we need to connect everything (populating left_circuit_elements and right_circuit_elements vecs)
        //let's start by connecting the mass capacitors to their grounds
        for (size_t mass_index=0; mass_index<input_mechanical_circuit.mass_list_.size();mass_index++){
            MassObject mass_element=input_mechanical_circuit.mass_list_[mass_index];
            ElectricalCircuitElement mass_capacitor=circuit_element_map_.at(mass_element.name_);
            ElectricalCircuitElement mass_capacitor_ground=circuit_element_map_.at(mass_element.name_+"_ground");
            mass_capacitor.add_right_element(mass_capacitor_ground);
            mass_capacitor_ground.add_left_element(mass_capacitor);

            //update objects
            circuit_element_map_.at(mass_element.name_)=mass_capacitor;
            circuit_element_map_.at(mass_element.name_+"_ground")=mass_capacitor_ground;

            int elem_index=std::distance(circuit_element_list_.begin(), std::find(circuit_element_list_.begin(),circuit_element_list_.end(),mass_capacitor));
            int ground_elem_index=std::distance(circuit_element_list_.begin(), std::find(circuit_element_list_.begin(),circuit_element_list_.end(),mass_capacitor_ground));

            circuit_element_list_[elem_index]=mass_capacitor;
            circuit_element_list_[ground_elem_index]=mass_capacitor_ground;
        }
        //now let's do something similar for force objects
        for (size_t force_index=0; force_index<input_mechanical_circuit.force_list_.size();force_index++){
            ForceObject force_element=input_mechanical_circuit.force_list_[force_index];
            ElectricalCircuitElement force_curr_source=circuit_element_map_.at(force_element.name_);
            ElectricalCircuitElement force_curr_source_ground=circuit_element_map_.at(force_element.name_+"_ground");
            force_curr_source.add_left_element(force_curr_source_ground);
            force_curr_source_ground.add_right_element(force_curr_source);

            //update objects
            circuit_element_map_.at(force_element.name_)=force_curr_source;
            circuit_element_map_.at(force_element.name_+"_ground")=force_curr_source_ground;

            int elem_index=std::distance(circuit_element_list_.begin(), std::find(circuit_element_list_.begin(),circuit_element_list_.end(),force_curr_source));
            int ground_elem_index=std::distance(circuit_element_list_.begin(), std::find(circuit_element_list_.begin(),circuit_element_list_.end(),force_curr_source_ground));

            circuit_element_list_[elem_index]=force_curr_source;
            circuit_element_list_[ground_elem_index]=force_curr_source_ground;
        }


        //let's loop back over connection objects again (there's almost certainly a more efficient way to do this, but this'll suffice for now)
        for (size_t connection_index=0; connection_index<input_mechanical_circuit.connection_list_.size();connection_index++){
            ConnectionObject connection_element=input_mechanical_circuit.connection_list_[connection_index];
            //first looking at the element it's connected to on its left
            //if that object isn't already listed as a connection to its left, add it (it might already be listed if, e.g., it was connected to another connection object, like a spring connected to a damper, and that other connection object got processed first)
            ElectricalCircuitElement element=circuit_element_map_.at(connection_element.name_);
            ElectricalCircuitElement element_to_left=circuit_element_map_.at(connection_element.left_element_name_);
            ElectricalCircuitElement element_to_right=circuit_element_map_.at(connection_element.right_element_name_);

            auto iter = std::find(element.left_circuit_elements.begin(),element.left_circuit_elements.end(),element_to_left);

            if (iter==element.left_circuit_elements.end()){
                //if it's not already there
                element.add_left_element(element_to_left);
            }
            //and add this connection object as a right element of whatever it was connecting to, if it's not already there
            iter = std::find(element_to_left.right_circuit_elements.begin(),element_to_left.right_circuit_elements.end(),element);

            if (iter==element_to_left.right_circuit_elements.end()){
                //if it's not already there
                element_to_left.add_right_element(element);
            }

            //now same for the connection to the right side of the element being looked at 
            iter = std::find(element.right_circuit_elements.begin(),element.right_circuit_elements.end(),element_to_right);

            if (iter==element.right_circuit_elements.end()){
                //if it's not already there
                element.add_right_element(element_to_right);
            }

            iter = std::find(element_to_right.left_circuit_elements.begin(),element_to_right.left_circuit_elements.end(),element);

            if (iter==element_to_right.left_circuit_elements.end()){
                //if it's not already there
                element_to_right.add_left_element(element);
            }

            //update circuit element objects in circuit list and map
            circuit_element_map_.at(connection_element.name_)=element;
            circuit_element_map_.at(connection_element.left_element_name_)=element_to_left;
            circuit_element_map_.at(connection_element.right_element_name_)=element_to_right;

            int elem_index=std::distance(circuit_element_list_.begin(), std::find(circuit_element_list_.begin(),circuit_element_list_.end(),element));
            int left_elem_index=std::distance(circuit_element_list_.begin(), std::find(circuit_element_list_.begin(),circuit_element_list_.end(),element_to_left));
            int right_elem_index=std::distance(circuit_element_list_.begin(), std::find(circuit_element_list_.begin(),circuit_element_list_.end(),element_to_right));

            circuit_element_list_.at(elem_index)=element;
            circuit_element_list_.at(left_elem_index)=element_to_left;
            circuit_element_list_.at(right_elem_index)=element_to_right;
        }
    }

    void draw_electrical_circuit(){
        //first we need to know how many rows we need to draw. This is driven by the maximum number of elements in parallel seen across the whole circuit.
        //we can determine the maximum number of elements connected in parallel by the maximum number of elements connected to the left or right of any other element
        //e.g., if all elements have at most one other element they are connected to on their left and right, it's a serial circuit
        int num_rows=1;
        //basic idea is to look through all the elements and find the max length of their left and right vectors

        for (size_t element_index=0; element_index<circuit_element_list_.size();element_index++){
            ElectricalCircuitElement tmp_element=circuit_element_list_[element_index];
            std::cout << "looking at circuit element named " << tmp_element.name_ << "\n";
            std::cout << "here are the names of the elements it's connected to on its left:\n";
            for (size_t left_index=0; left_index<tmp_element.left_circuit_elements.size();left_index++){
                std::cout << tmp_element.left_circuit_elements[left_index].name_ << "\n";
            }
            std::cout << "here are the names of the elements it's connected to on its right:\n";

            for (size_t right_index=0; right_index<tmp_element.right_circuit_elements.size();right_index++){
                std::cout << tmp_element.right_circuit_elements[right_index].name_ << "\n";
            }
            int left_size=tmp_element.left_circuit_elements.size();
            int right_size=tmp_element.right_circuit_elements.size();
            if (left_size>num_rows){
                num_rows=left_size;
            }
            if (right_size>num_rows){
                num_rows=right_size;
            }
        }
        std::cout << "num rows: " << num_rows << "\n";

    }



    // void force_voltage_conversion(MechanicalCircuit input_mechanical_circuit){
    //     //this version is using the force-voltage analogy
    //     //not done yet
    //     //first, convert masses to inductors
    //     for (size_t mass_index=0; mass_index<input_mechanical_circuit.mass_list_.size();mass_index++){
    //         MassObject mass_element=input_mechanical_circuit.mass_list_[mass_index];
    //         //add input node
    //         add_node(mass_element.name_+"_node_left");
    //         //add output node
    //         add_node(mass_element.name_+"_node_right");
    //         add_circuit_element(mass_element.name_,"inductor",mass_element.name_+"_node_left",mass_element.name_+"_node_right",mass_element.mass_);
    //     }

    //     //next, convert dampers to resistors
    //     for (size_t connection_index=0; connection_index<input_mechanical_circuit.connection_list_.size();connection_index++){
    //         ConnectionObject connection_element=input_mechanical_circuit.connection_list_[connection_index];
    //         if (connection_element.type_.compare("damper")==0){
    //         //add input node
    //         add_node(connection_element.name_+"_node_left");
    //         //add output node
    //         add_node(connection_element.name_+"_node_right");
    //         add_circuit_element(connection_element.name_,"resistor",connection_element.name_+"_node_left",connection_element.name_+"_node_right",connection_element.coefficient_);
    //         }
    //         //next convert springs to capacitors
    //         else if (connection_element.type_.compare("spring")==0){
    //         //add input node
    //         add_node(connection_element.name_+"_node_left");
    //         //add output node
    //         add_node(connection_element.name_+"_node_right");
    //         add_circuit_element(connection_element.name_,"capacitor",connection_element.name_+"_node_left",connection_element.name_+"_node_right",1/connection_element.coefficient_);
    //         }
    //     }
    //     //now convert forces to batteries (voltage sources)
    //     for (size_t force_index=0; force_index<input_mechanical_circuit.force_list_.size();force_index++){
    //         ForceObject force_element=input_mechanical_circuit.force_list_[force_index];
    //         //figure out conversion between sign on force and orientation of positive/negative terminals on battery
    //         //add input node
    //         add_node(force_element.name_+"_node_left");
    //         //add output node
    //         add_node(force_element.name_+"_node_right");
    //         add_circuit_element(force_element.name_,"battery",force_element.name_+"_node_left",force_element.name_+"_node_right",force_element.force_val_);
    //     }

    //     //now need to connect elements properly via wire objects
    // }
};
