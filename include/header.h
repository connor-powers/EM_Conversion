#include <iostream>
#include <Eigen/Dense>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <cmath>

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

        bool operator==(const ElectricalCircuitElement& other_element) const {
            return (name_==other_element.name_);
        }
};

class ElectricalCircuit
{
    private:
        std::vector<ElectricalCircuitElement> circuit_element_list_{};
        std::string first_force_name_="";
    public:
        ElectricalCircuit(MechanicalCircuit input_mechanical_circuit,std::string analogy){
            if (analogy.compare("Force-Current")==0){
                force_current_conversion(input_mechanical_circuit);
            }
        }
    
    void add_circuit_element(std::string element_name,std::string element_type,double coefficient){
        ElectricalCircuitElement new_element(element_name,element_type,coefficient);
        circuit_element_list_.push_back(new_element);
    }
    
    void connect_elements(std::string left_element_name, std::string right_element_name){
        auto lambdafunc_right = [right_element_name](ElectricalCircuitElement& tmp_element) {
            return (tmp_element.name_.compare(right_element_name)==0);
        };
        auto lambdafunc_left = [left_element_name](ElectricalCircuitElement& tmp_element) {
            return (tmp_element.name_.compare(left_element_name)==0);
        };
        int right_elem_index=std::distance(circuit_element_list_.begin(), std::find_if(circuit_element_list_.begin(),circuit_element_list_.end(),lambdafunc_right));
        int left_elem_index=std::distance(circuit_element_list_.begin(), std::find_if(circuit_element_list_.begin(),circuit_element_list_.end(),lambdafunc_left));
        ElectricalCircuitElement left_element=circuit_element_list_.at(left_elem_index);
        ElectricalCircuitElement right_element=circuit_element_list_.at(right_elem_index);

        //now actually add the element
        right_element.left_circuit_elements.push_back(left_element);
        left_element.right_circuit_elements.push_back(right_element);

        circuit_element_list_.at(left_elem_index)=left_element;
        circuit_element_list_.at(right_elem_index)=right_element;


    }

    int find_element_index(std::string element_name){
        auto lambdafunc = [element_name](ElectricalCircuitElement& tmp_element) {
            return (tmp_element.name_.compare(element_name)==0);
        };
        auto iterator = std::find_if(circuit_element_list_.begin(),circuit_element_list_.end(),lambdafunc);
        if (iterator != circuit_element_list_.end()){
            return std::distance(circuit_element_list_.begin(),iterator);
        }
        else {
            std::cout << "didn't find input element name " << element_name << "in circuit element list\n";
            return -1;
        }
    }


    void force_current_conversion(MechanicalCircuit input_mechanical_circuit){
        //this version is using the force-current analogy
        //first, convert masses to grounded capacitors

        for (size_t mass_index=0; mass_index<input_mechanical_circuit.mass_list_.size();mass_index++){
            MassObject mass_element=input_mechanical_circuit.mass_list_[mass_index];
            if (mass_element.name_.compare("ground")!=0){
                add_circuit_element(mass_element.name_+"_ground","ground",0);
            }
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
                add_circuit_element(connection_element.name_,"inductor",1/connection_element.coefficient_);
            }
        }
        //now convert forces to current sources and connect it to the corresponding circuit element and a ground (assuming ground is to the left of forces for now)
        for (size_t force_index=0; force_index<input_mechanical_circuit.force_list_.size();force_index++){
            ForceObject force_element=input_mechanical_circuit.force_list_[force_index];
            add_circuit_element(force_element.name_+"_ground","ground",0);
            add_circuit_element(force_element.name_,"current_source",force_element.force_val_);
            if (force_index==0){
                first_force_name_=force_element.name_;
            }
        }

        //now we need to connect everything (populating left_circuit_elements and right_circuit_elements vecs)
        //let's start by connecting the mass capacitors to their grounds
        for (size_t mass_index=0; mass_index<input_mechanical_circuit.mass_list_.size();mass_index++){
            MassObject mass_element=input_mechanical_circuit.mass_list_[mass_index];
            if (mass_element.name_.compare("ground")!=0){
                connect_elements(mass_element.name_,mass_element.name_+"_ground");
            }
        }
        //now let's do something similar for force objects
        for (size_t force_index=0; force_index<input_mechanical_circuit.force_list_.size();force_index++){
            ForceObject force_element=input_mechanical_circuit.force_list_[force_index];
            connect_elements(force_element.name_+"_ground",force_element.name_);
            connect_elements(force_element.name_,force_element.object_name_);
        }

        //let's loop back over connection objects again (there's almost certainly a more efficient way to do this, but this'll suffice for now)
        for (size_t connection_index=0; connection_index<input_mechanical_circuit.connection_list_.size();connection_index++){
            ConnectionObject connection_element=input_mechanical_circuit.connection_list_[connection_index];
            connect_elements(connection_element.name_,connection_element.right_element_name_);
            connect_elements(connection_element.left_element_name_,connection_element.name_);
        }
    }


    void visit_element(ElectricalCircuitElement current_elem,std::vector<std::vector<std::string>>& input_circuit_grid, std::vector<std::string> input_circuit_grid_row, int input_row_index, int input_column_index,std::vector<std::string>& input_visited_elements,int input_padded_string_length){
        std::cout << "starting search at element " << current_elem.name_ << "\n";
        std::cout << "adding " << current_elem.name_ << " to row index " << input_row_index << " and column index " << input_column_index << "\n";
        std::string disp_string="";
        if ((current_elem.name_.compare("ground")==0)||(current_elem.type_.compare("ground")==0)){
            disp_string="[G]";
        }
        else {
            std::string prefix="";
            std::ostringstream stringstream;
            stringstream << std::fixed << std::setprecision(3) << current_elem.coefficient_;
            disp_string=stringstream.str()+")";
            if (current_elem.type_=="resistor"){
                prefix="R(";
            }
            else if (current_elem.type_=="capacitor"){
                prefix="C(";
            }
            else if (current_elem.type_=="inductor"){
                prefix="I(";
            }
            else if (current_elem.type_=="current_source"){
                prefix="[->](";
            }
            disp_string=prefix+disp_string;
        }
        input_circuit_grid.at(input_row_index).at(input_column_index)=disp_string;
        input_visited_elements.push_back(current_elem.name_);

        //first exploring left

        if (current_elem.left_circuit_elements.size()>0){
            std::cout << "found " << current_elem.left_circuit_elements.size() <<  " element(s) to the left, exploring that way first\n";
            for (int lhs_index=0;lhs_index<current_elem.left_circuit_elements.size();lhs_index++){
                ElectricalCircuitElement new_elem=circuit_element_list_.at(find_element_index(current_elem.left_circuit_elements.at(lhs_index).name_));
                if (std::find(input_visited_elements.begin(),input_visited_elements.end(),new_elem.name_)!=input_visited_elements.end()){
                    //if that element has already been explored, skip it
                    std::cout << "already processed that one, skipping it\n";
                    continue;
                }
                if ((lhs_index==0)&&(current_elem.left_circuit_elements.size()==0)){
                    if (new_elem.name_.compare(input_circuit_grid.at(input_row_index).at(input_column_index-1))!=0){
                        std::cout << "Huh?";
                    }
                }
                else {

                    if (lhs_index>0){
                        std::cout << "couldn't find an element name of " << new_elem.name_ << " in input grid, so adding a row\n";
                        input_circuit_grid.push_back(input_circuit_grid_row);
                        std::cout << "adding a branch collapse indicator at row " << input_row_index+1 << " and column index " << input_column_index << "\n";

                        input_circuit_grid.at(input_row_index+1).at(input_column_index)="b.c.u.i.t.c.";//branch collapsing up into this column
                        input_circuit_grid.push_back(input_circuit_grid_row);

                    }
                    visit_element(new_elem,input_circuit_grid,input_circuit_grid_row,input_row_index+(2*lhs_index),input_column_index-1,input_visited_elements,input_padded_string_length);

                }
            }

        }

        //now explore right
        if (current_elem.right_circuit_elements.size()>0){
            std::cout << "found " << current_elem.right_circuit_elements.size() <<  " element(s) to the right, exploring that way\n";
            int effective_index=0;
            for (int rhs_index=0;rhs_index<current_elem.right_circuit_elements.size();rhs_index++){
                ElectricalCircuitElement new_elem=circuit_element_list_.at(find_element_index(current_elem.right_circuit_elements.at(rhs_index).name_));
                std::cout << "found an elem on the right called " << new_elem.name_ << "\n";
                if (std::find(input_visited_elements.begin(),input_visited_elements.end(),new_elem.name_)!=input_visited_elements.end()){
                    //if that element has already been explored, skip it
                    std::cout << "already processed that one, skipping it\n";
                    continue;
                }

                if ((rhs_index==0)&&(input_circuit_grid.at(input_row_index).at(input_column_index+1).size()>0)){
                    std::cout << "Already an element there, iterating effective index to avoid overwriting\n";
                    effective_index++;
                }
                if (effective_index>0){
                    std::cout << "couldn't find an element name of " << new_elem.name_ << " in input grid, so adding a row\n";
                    input_circuit_grid.push_back(input_circuit_grid_row);
                    std::cout << "adding a branch expansion indicator at row " << input_row_index+1 << " and column index " << input_column_index << "\n";

                    input_circuit_grid.at(input_row_index+1).at(input_column_index)="b.e.o.f.t.c.";//branch expanding out from this column
                    input_circuit_grid.push_back(input_circuit_grid_row);                    
                }
                visit_element(new_elem,input_circuit_grid,input_circuit_grid_row,input_row_index+(2*effective_index),input_column_index+1,input_visited_elements,input_padded_string_length);
                effective_index++;

                  
            }

        }
    }


    void draw_electrical_circuit(){
        //start with the ground of the first force. This will be the top left of the visualized circuit.
        std::vector<std::string> circuit_grid_row(circuit_element_list_.size()+1,"");
        std::vector<std::vector<std::string>> circuit_grid;
        std::vector<std::string> visited_elements;


        circuit_grid.push_back(circuit_grid_row);//first row
        int current_row_index=0;
        int current_column_index=0;

        //now let's start our general loop.
        ElectricalCircuitElement first_elem=circuit_element_list_.at(find_element_index(first_force_name_+"_ground"));
        int padded_string_length= 14;

        visit_element(first_elem,circuit_grid,circuit_grid_row,current_row_index,current_column_index,visited_elements,padded_string_length);
        std::string branch_expansion_string="           | ";
        std::string branch_collapse_string="          |  ";
        std::cout << "\n";

        for (int row_ind=0;row_ind<circuit_grid.size();row_ind++){
            std::string row_str="";
            for (int col_ind=0;col_ind<circuit_grid.at(0).size();col_ind++){
                std::string current_string=circuit_grid.at(row_ind).at(col_ind);

                if (circuit_grid.at(row_ind).at(col_ind).compare("b.e.o.f.t.c.")==0){
                    //so there's a branch expanding out from the above element
                    //to do: make this more flexible
                    current_string=branch_expansion_string;
                }

                if ((row_ind>0)&&(col_ind>0)&&(circuit_grid.at(row_ind-1).at(col_ind).compare("b.e.o.f.t.c.")==0)){
                    //so there's a branch expanding out from the above element
                    //to do: make this more flexible
                    current_string="           --";

                }
                if ((col_ind<circuit_grid.at(0).size()-1)&&(circuit_grid.at(row_ind).at(col_ind+1).compare("b.c.u.i.t.c.")==0)){
                    //so there's a branch collapsing into the above element
                    //to do: make this more flexible
                    current_string=branch_expansion_string;
                }
                if ((row_ind>0)&&(col_ind<circuit_grid.at(0).size()-1)&&(circuit_grid.at(row_ind-1).at(col_ind+1).compare("b.c.u.i.t.c.")==0)){
                    //so there's a branch collapsing into the above element
                    //to do: make this more flexible
                    current_string+="----";
                }
                if (current_string.compare("b.c.u.i.t.c.")==0){

                    current_string="";
                }

                if ((col_ind>0)&&(col_ind<circuit_grid.at(0).size()-1)){
                    std::string right_string=circuit_grid.at(row_ind).at(col_ind+1);
                    std::string left_string=circuit_grid.at(row_ind).at(col_ind-1);
                    if ((current_string.size()>0)&&(right_string.size()>0)&&(right_string.compare("b.e.o.f.t.c.")!=0)&&(right_string.compare("b.c.u.i.t.c.")!=0)){
                        //elems on both sides, should be wire between right? might want to go back and add some kind of check on this
                        while (current_string.size()<padded_string_length){
                            current_string+="-";
                        }
                    }
                    else {
                        while (current_string.size()<padded_string_length){
                            current_string+=" ";
                        }
                    }
                }

                else if (col_ind==0){
                    std::string right_string=circuit_grid.at(row_ind).at(col_ind+1);
                    if (right_string.size()>0){
                        while (current_string.size()<padded_string_length){
                            current_string+="-";
                        }
                    }
                    else {
                        while (current_string.size()<padded_string_length){
                            current_string+=" ";
                        }
                    }

                }

                row_str = row_str+current_string;

            }
            std::cout << row_str << "\n";
        }

        std::cout << "\nLegend:\n";
        std::cout << "[G]: Ground\n";
        std::cout << "[C](x): Capacitor with capacitance x\n";
        std::cout << "[R](x): Resistor with resistance x\n";
        std::cout << "[I](x): Inductor with inductance x\n";
        std::cout << "[->](x): Current source with current x\n";
    }
};
