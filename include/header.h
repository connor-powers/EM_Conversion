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
        std::string left_object_name_="";
        std::string right_object_name_="";
        double coefficient_={0};

        ConnectionObject(std::string name, std::string type,std::string left_object_name,std::string right_object_name, double coefficient){
            name_=name;
            type_=type;
            left_object_name_=left_object_name;
            right_object_name_=right_object_name;
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
            std::string left_object_name;
            std::string right_object_name;
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
                        add_connection(element_name,element_connection_type,left_object_name,right_object_name,element_connection_coefficient);
                    }
                    else if (element_type.compare("force")==0){
                        add_force(element_name,object_name,element_connection_coefficient);
                    }
                    
                    element_name="";
                    element_mass=0;
                    element_MOI=0;
                    element_connection_type="";
                    left_object_name="";
                    right_object_name="";
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
                    else if (key.compare("left_object")==0){
                        left_object_name=val;
                    }
                    else if (key.compare("right_object")==0){
                        right_object_name=val;
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
            add_connection(element_name,element_connection_type,left_object_name,right_object_name,element_connection_coefficient);
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

    void add_connection(std::string name, std::string type,std::string left_object_name,std::string right_object_name, double coefficient){
        ConnectionObject connection_object=ConnectionObject(name,type,left_object_name,right_object_name,coefficient);
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
            std::cout << "Object on left of connection: " << tmp.left_object_name_ << "\n";
            std::cout << "Object on right of connection: " << tmp.right_object_name_ << "\n";
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
                        if ((tmp_connection_obj.left_object_name_==left_obj_name) && (tmp_connection_obj.right_object_name_==right_obj_name)){
                            (connection_matrix_.at(0))(left_obj_index,right_obj_index)=tmp_connection_obj.coefficient_;
                            (connection_matrix_.at(1))(left_obj_index,right_obj_index)=std::distance(connection_type_list.begin(), std::find(connection_type_list.begin(),connection_type_list.end(),tmp_connection_obj.type_));
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
        std::string left_node_name_="";
        std::string right_node_name_="";
        double resistance_=0.0;

        ResistorObject(std::string name,std::string left_node_name, std::string right_node_name, double resistance){
            name_=name;
            left_node_name_=left_node_name;
            right_node_name_=right_node_name;
            resistance_=resistance;
        }
};

class CapacitorObject
{

    public:
        std::string name_="";
        std::string left_node_name_="";
        std::string right_node_name_="";
        double capacitance_=0.0;

        CapacitorObject(std::string name,std::string left_node_name, std::string right_node_name, double capacitance){
            name_=name;
            left_node_name_=left_node_name;
            right_node_name_=right_node_name;
            capacitance_=capacitance;
        }
};

class InductorObject
{
    public:
        std::string name_="";
        std::string left_node_name_="";
        std::string right_node_name_="";
        double inductance_=0.0;

        InductorObject(std::string name,std::string left_node_name, std::string right_node_name, double inductance){
            name_=name;
            left_node_name_=left_node_name;
            right_node_name_=right_node_name;
            inductance_=inductance;
        }
};

class NodeObject
{
    public:
        std::string name_="";

        NodeObject(std::string name){
            name_=name;
        }
};

class WireObject
{
    public:
        std::string left_node_name_="";
        std::string right_node_name_="";

        WireObject(std::string left_node_name,std::string right_node_name,std::vector<NodeObject>& node_list){
            //verify that both input nodes exist in node list
            bool left_found_flag=false;
            bool right_found_flag=false;
            for (size_t node_index=0;node_index<node_list.size();node_index++){
                NodeObject tmp_node=node_list[node_index];
                if (tmp_node.name_.compare(left_node_name)==0){
                    left_found_flag=true;
                }
                if (tmp_node.name_.compare(right_node_name)==0){
                    right_found_flag=true;
                }
            }
            if (left_found_flag==false){
                std::cout << "When drawing a wire between nodes "<< left_node_name <<" and " << right_node_name << ", couldn't find node labeled " << left_node_name << "\n";
            }
            if (right_found_flag==false){
                std::cout << "When drawing a wire between nodes "<< left_node_name <<" and " << right_node_name << ", couldn't find node labeled " << right_node_name << "\n";
            }
            left_node_name_=left_node_name;
            right_node_name_=right_node_name;
        }
};

class BatteryObject
{
    public:
        std::string name_="";
        std::string left_node_name_="";
        std::string right_node_name_="";
        double voltage_=0.0;

        BatteryObject(std::string name,std::string left_node_name, std::string right_node_name, double voltage){
            name_=name;
            left_node_name_=left_node_name;
            right_node_name_=right_node_name;
            voltage_=voltage;
        }
};

class CurrentSourceObject
{
    public:
        std::string name_="";
        std::string left_node_name_="";
        std::string right_node_name_="";
        double current_=0.0;

        CurrentSourceObject(std::string name,std::string left_node_name, std::string right_node_name, double current){
            name_=name;
            left_node_name_=left_node_name;
            right_node_name_=right_node_name;
            current_=current;
        }
};

class ElectricalCircuit
{
    private:
        std::vector<ResistorObject> resistor_list_{};
        std::vector<CapacitorObject> capacitor_list_{};
        std::vector<InductorObject> inductor_list_{};
        std::vector<NodeObject> node_list_{};
        std::vector<BatteryObject> battery_list_{};
        std::vector<CurrentSourceObject> current_source_list_{};
        std::vector<WireObject> wire_list_{};
        std::vector<std::string> mech_connection_type_list={"self","damper","spring"};

    public:
        ElectricalCircuit(MechanicalCircuit input_mechanical_circuit,std::string analogy){
            if (analogy.compare("Force-Current")==0){
                force_current_conversion(input_mechanical_circuit);
            }
        }
    
    void add_circuit_element(std::string element_type,std::string element_name,std::string left_node_name,std::string right_node_name,double coefficient){
        if (element_type.compare("resistor")==0){
            ResistorObject new_resistor(element_name,left_node_name,right_node_name,coefficient);
            resistor_list_.insert(resistor_list_.end(),new_resistor);
        }
        else if (element_type.compare("capacitor")==0){
            CapacitorObject new_capacitor(element_name,left_node_name,right_node_name,coefficient);
            capacitor_list_.insert(capacitor_list_.end(),new_capacitor);
        }
        else if (element_type.compare("inductor")==0){
            InductorObject new_inductor(element_name,left_node_name,right_node_name,coefficient);
            inductor_list_.insert(inductor_list_.end(),new_inductor);
        }

        else if (element_type.compare("battery")==0){
            BatteryObject new_battery(element_name,left_node_name,right_node_name,coefficient);
            battery_list_.insert(battery_list_.end(),new_battery);
        }
        else if (element_type.compare("current_source")==0){
            CurrentSourceObject new_current_source(element_name,left_node_name,right_node_name,coefficient);
            current_source_list_.insert(current_source_list_.end(),new_current_source);
        }
    }

    void add_node(std::string node_name){
        NodeObject new_node(node_name);
        node_list_.insert(node_list_.end(),new_node);
    }

    void add_wire(std::string left_node_name,std::string right_node_name,std::vector<NodeObject>& node_list){
        WireObject new_wire(left_node_name,right_node_name,node_list);
        wire_list_.insert(wire_list_.end(),new_wire);
    }

    void list_nodes(){
        for (size_t node_index=0;node_index<node_list_.size();node_index++){
            NodeObject tmp_node=node_list_[node_index];
            std::cout << "Node labeled " << tmp_node.name_ << "\n";
        }
    }


    void force_current_conversion(MechanicalCircuit input_mechanical_circuit){
        //this version is using the force-current analogy
        //first, convert masses to grounded capacitors
        for (size_t mass_index=0; mass_index<input_mechanical_circuit.mass_list_.size();mass_index++){
            MassObject mass_element=input_mechanical_circuit.mass_list_[mass_index];
            //add input node
            add_node(mass_element.name_+"_node_left");
            //add output node and wire it to ground
            add_node(mass_element.name_+"_node_right");
            add_node(mass_element.name_+"_ground");
            add_wire(mass_element.name_+"_node_right",mass_element.name_+"_ground",node_list_);
            add_circuit_element(mass_element.name_,"capacitor",mass_element.name_+"_node_left",mass_element.name_+"_ground",mass_element.mass_);
        }

        for (size_t connection_index=0; connection_index<input_mechanical_circuit.connection_list_.size();connection_index++){
            //next, convert dampers to resistors
            ConnectionObject connection_element=input_mechanical_circuit.connection_list_[connection_index];
            if (connection_element.type_.compare("damper")==0){
            //add input node
            add_node(connection_element.name_+"_node_left");
            //add output node
            add_node(connection_element.name_+"_node_right");
            add_circuit_element(connection_element.name_,"resistor",connection_element.name_+"_node_left",connection_element.name_+"_node_right",1/connection_element.coefficient_);
            }
            //next convert springs to inductors
            else if (connection_element.type_.compare("spring")==0){
            //add input node
            add_node(connection_element.name_+"_node_left");
            //add output node
            add_node(connection_element.name_+"_node_right");
            add_circuit_element(connection_element.name_,"capacitor",connection_element.name_+"_node_left",connection_element.name_+"_node_right",1/connection_element.coefficient_);
            }
        }
        //now convert forces to current sources
        for (size_t force_index=0; force_index<input_mechanical_circuit.force_list_.size();force_index++){
            ForceObject force_element=input_mechanical_circuit.force_list_[force_index];
            //add input node
            add_node(force_element.name_+"_node_left");
            //going to wire "right_node" of masses to ground for externally applied forces, direction of force controls sign of current
            add_node(force_element.name_+"_node_right");
            add_node(force_element.name_+"_ground");
            add_wire(force_element.name_+"_node_right",force_element.name_+"_ground",node_list_);
            add_circuit_element(force_element.name_,"current_source",force_element.name_+"_node_left",force_element.name_+"_ground",force_element.force_val_);
        }
        list_nodes();
        //now need to connect elements properly via wire objects
        for (size_t connection_index=0; connection_index<input_mechanical_circuit.connection_list_.size();connection_index++){
            ConnectionObject connection_element=input_mechanical_circuit.connection_list_[connection_index];
            add_wire(connection_element.left_object_name_+"_node_right",connection_element.name_+"_node_left",node_list_);
            add_wire(connection_element.name_+"_node_right",connection_element.right_object_name_+"_node_left",node_list_);
        }
    }

    int find_node_index_by_name(std::string node_name){
        //identify the index of then named node in the current node_list_
        auto iterator = std::find_if(node_list_.begin(), node_list_.end(), [&node_name](NodeObject tmp_node) {return (tmp_node.name_.compare(node_name)==0);});

        if (iterator != node_list_.end())
        {
            //if it found something
            int matched_index = std::distance(node_list_.begin(), iterator);
            return matched_index;
        }
        else {
            std::cout << "couldn't find index of node with name " << node_name << "\n";
            return -1;
        }
    }

    int find_capacitor_index_by_name(std::string capacitor_name){
        //identify the index of then named capacitor in the current capacitor_list_
        auto iterator = std::find_if(capacitor_list_.begin(), capacitor_list_.end(), [&capacitor_name](CapacitorObject tmp_capacitor) {return (tmp_capacitor.name_.compare(capacitor_name)==0);});

        if (iterator != capacitor_list_.end())
        {
            //if it found something
            int matched_index = std::distance(capacitor_list_.begin(), iterator);
            return matched_index;
        }
        else {
            std::cout << "couldn't find index of capacitor with name " << capacitor_name << "\n";
            return -1;
        }
    }

    int find_resistor_index_by_name(std::string resistor_name){
        //identify the index of then named resistor in the current resistor_list_
        auto iterator = std::find_if(resistor_list_.begin(), resistor_list_.end(), [&resistor_name](ResistorObject tmp_resistor) {return (tmp_resistor.name_.compare(resistor_name)==0);});

        if (iterator != resistor_list_.end())
        {
            //if it found something
            int matched_index = std::distance(resistor_list_.begin(), iterator);
            return matched_index;
        }
        else {
            std::cout << "couldn't find index of resistor with name " << resistor_name << "\n";
            return -1;
        }
    }

    int find_inductor_index_by_name(std::string inductor_name){
        //identify the index of then named inductor in the current inductor_list_
        auto iterator = std::find_if(inductor_list_.begin(), inductor_list_.end(), [&inductor_name](InductorObject tmp_inductor) {return (tmp_inductor.name_.compare(inductor_name)==0);});

        if (iterator != inductor_list_.end())
        {
            //if it found something
            int matched_index = std::distance(inductor_list_.begin(), iterator);
            return matched_index;
        }
        else {
            std::cout << "couldn't find index of inductor with name " << inductor_name << "\n";
            return -1;
        }
    }

    void draw_electrical_circuit(){
        //first we need to know how many rows we need to draw. This is driven by the maximum number of elements in parallel seen across the whole circuit.
        //we can determine the maximum number of elements connected in parallel by the maximum number of elements going in or out of any given node in the circuit.
        int num_rows=1;
        //basic idea is to look through all the wires and tally up how many incoming and outgoing wires are associated with each node
        //can make two zeroed vectors of the same length as the node list, one to keep track of each node's incoming connections and one to keep track of its outgoing connections
        //incoming is taken to be leftmost node and outgoing is taken to be rightmost node (reading circuits left to right)
        std::vector<int> node_incoming_connections(node_list_.size());
        std::vector<int> node_outgoing_connections(node_list_.size());

        for (size_t wire_index=0; wire_index<wire_list_.size();wire_index++){
            WireObject tmp_wire=wire_list_[wire_index];
            std::cout << "looking at wire between "<<tmp_wire.left_node_name_<< " and "<<tmp_wire.right_node_name_ << "\n";
            int left_node_index=find_node_index_by_name(tmp_wire.left_node_name_);
            std::cout << "left node index: " << left_node_index;
            int right_node_index=find_node_index_by_name(tmp_wire.right_node_name_);
            std::cout << "right node index: " << right_node_index;
            int incoming_current_val;
            int outgoing_current_val;
            if (left_node_index==-1){
                std::cout << "skipping past this wire between "<<tmp_wire.left_node_name_<< " and "<<tmp_wire.right_node_name_ << " because of invalid node index on left node\n";
                continue;
            }
            if (right_node_index==-1){
                std::cout << "skipping past this wire between "<<tmp_wire.left_node_name_<< " and "<<tmp_wire.right_node_name_ << " because of invalid node index on right node\n";
                continue;
            }
            incoming_current_val=node_incoming_connections.at(left_node_index);
            node_incoming_connections.at(left_node_index)=incoming_current_val+1;
            outgoing_current_val=node_outgoing_connections.at(right_node_index);
            node_outgoing_connections.at(right_node_index)=outgoing_current_val+1;
        }
        int max_incoming_vec=*std::max_element(node_incoming_connections.begin(),node_incoming_connections.end());
        int max_outgoing_vec=*std::max_element(node_outgoing_connections.begin(),node_outgoing_connections.end());
        if (max_incoming_vec>=max_outgoing_vec){
            num_rows= max_incoming_vec;
        }
        else if (max_incoming_vec<max_outgoing_vec){
            num_rows= max_outgoing_vec;
        }
        std::cout << "incoming connection vec: ";
        for ( int index{0}; index < node_incoming_connections.size(); ++index ){ 
            std::cout << node_incoming_connections.at(index) << " ";
        }
        std::cout << "\n outgoing connection vec: ";
        for ( int index{0}; index < node_outgoing_connections.size(); ++index ){ 
            std::cout << node_outgoing_connections.at(index) << " ";
        }        std::cout << "\n number of rows: " << num_rows<<"\n";


        //now how do we check what elements can be drawn first on the left?
        //first idea is to check if there are any elements which aren't listed as the right-side object in a connection (forces don't count)
        //recall connections can also be elements (e.g., a damper connected in series to a spring)
        std::vector<bool> capacitor_bool_vec(capacitor_list_.size(),true);
        std::vector<bool> resistor_bool_vec(resistor_list_.size(),true);
        std::vector<bool> inductor_bool_vec(inductor_list_.size(),true);
        //now go through each connection (resistors and inductors here) and mark whatever element is on the right side as false in its respective list


    }


    void force_voltage_conversion(MechanicalCircuit input_mechanical_circuit){
        //this version is using the force-voltage analogy
        //not done yet
        //first, convert masses to inductors
        for (size_t mass_index=0; mass_index<input_mechanical_circuit.mass_list_.size();mass_index++){
            MassObject mass_element=input_mechanical_circuit.mass_list_[mass_index];
            //add input node
            add_node(mass_element.name_+"_node_left");
            //add output node
            add_node(mass_element.name_+"_node_right");
            add_circuit_element(mass_element.name_,"inductor",mass_element.name_+"_node_left",mass_element.name_+"_node_right",mass_element.mass_);
        }

        //next, convert dampers to resistors
        for (size_t connection_index=0; connection_index<input_mechanical_circuit.connection_list_.size();connection_index++){
            ConnectionObject connection_element=input_mechanical_circuit.connection_list_[connection_index];
            if (connection_element.type_.compare("damper")==0){
            //add input node
            add_node(connection_element.name_+"_node_left");
            //add output node
            add_node(connection_element.name_+"_node_right");
            add_circuit_element(connection_element.name_,"resistor",connection_element.name_+"_node_left",connection_element.name_+"_node_right",connection_element.coefficient_);
            }
            //next convert springs to capacitors
            else if (connection_element.type_.compare("spring")==0){
            //add input node
            add_node(connection_element.name_+"_node_left");
            //add output node
            add_node(connection_element.name_+"_node_right");
            add_circuit_element(connection_element.name_,"capacitor",connection_element.name_+"_node_left",connection_element.name_+"_node_right",1/connection_element.coefficient_);
            }
        }
        //now convert forces to batteries (voltage sources)
        for (size_t force_index=0; force_index<input_mechanical_circuit.force_list_.size();force_index++){
            ForceObject force_element=input_mechanical_circuit.force_list_[force_index];
            //figure out conversion between sign on force and orientation of positive/negative terminals on battery
            //add input node
            add_node(force_element.name_+"_node_left");
            //add output node
            add_node(force_element.name_+"_node_right");
            add_circuit_element(force_element.name_,"battery",force_element.name_+"_node_left",force_element.name_+"_node_right",force_element.force_val_);
        }

        //now need to connect elements properly via wire objects
    }
};
