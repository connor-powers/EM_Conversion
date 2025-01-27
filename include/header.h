#include <iostream>

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include <sstream>
#include <vector>

class MechanicalCircuitElement
{
    public:
        std::string name_="";
        std::string type_="";
        std::vector<std::string> left_circuit_element_names_; 
        std::vector<std::string> right_circuit_element_names_;
        double coefficient_=0.0;
        double moi_=0.0;

        MechanicalCircuitElement(std::string name, std::string type,double coefficient, double moi=0,std::string left_element_name="",std::string right_element_name=""){
            name_=name;
            type_=type;
            coefficient_=coefficient;
            if (type.compare("mass")==0){
                double moi_=moi;
            }
            if (left_element_name.size()>0){
                left_circuit_element_names_.push_back(left_element_name);
            }
            if (right_element_name.size()>0){
                right_circuit_element_names_.push_back(right_element_name);
            }
        }

        bool operator==(const MechanicalCircuitElement& other_element) const {
            return (name_==other_element.name_);
        }
};


class MechanicalCircuit
{
    private:

    public:
        std::vector<MechanicalCircuitElement> circuit_element_list_={};
        std::string first_force_name_="";
        bool verbosity_=false;
        MechanicalCircuit(std::string input_file_name){
            bool existing_element=false;
            std::ifstream input_file(input_file_name);
            std::string line;
            std::string element_type="";
            std::string element_name="";
            std::string left_element_name="";
            std::string right_element_name="";
            double coefficient=0;
            double moi=0;
            std::vector<std::string> list_of_element_names={};
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
                    if (element_name.size()>0){
                        if (existing_element==true){
                            std::cout << "Modifying " << element_type<< " named " << element_name << "\n";
                            //modifying existing element (likely adding another element that it's connected to on its left or right)
                            int relevant_index=find_element_index(element_name);
                            MechanicalCircuitElement tmp_element=circuit_element_list_.at(relevant_index);
                            if (std::find(tmp_element.left_circuit_element_names_.begin(),tmp_element.left_circuit_element_names_.end(),left_element_name)==tmp_element.left_circuit_element_names_.end()){
                                //element name isn't already in that list
                                tmp_element.left_circuit_element_names_.push_back(left_element_name);
                            }
                            if (std::find(tmp_element.right_circuit_element_names_.begin(),tmp_element.right_circuit_element_names_.end(),right_element_name)==tmp_element.right_circuit_element_names_.end()){
                                //element name isn't already in that list
                                tmp_element.right_circuit_element_names_.push_back(right_element_name);
                            }
                            circuit_element_list_.at(relevant_index)=tmp_element;
                        }
                        else {
                            std::cout << "Adding " << element_type << " with name " << element_name << "\n";
                            if (element_name.compare("ground")==0){
                                double infinity=INFINITY;
                                add_circuit_element(element_name,"mass",infinity,infinity);
                            }
                            else {
                                add_circuit_element(element_name,element_type,coefficient,moi,left_element_name,right_element_name);
                            }
                            
                            list_of_element_names.push_back(element_name);
                        }
                    }



                    element_name="";
                    element_type="";
                    coefficient=0;
                    moi=0;
                    left_element_name="";
                    right_element_name="";
                    existing_element=false;
                    

                    if (line=="@mass"){
                        element_type="mass";
                    }
                    else if (line=="@spring"){
                        element_type="spring";
                    }
                    else if (line=="@damper"){
                        element_type="damper";
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
                        element_name=val;
                        if (std::find(list_of_element_names.begin(),list_of_element_names.end(),val)!=list_of_element_names.end()){
                            existing_element=true;
                        }

                    }
                    else if ((key.compare("mass")==0)||(key.compare("coefficient")==0)||(key.compare("force_val")==0)){
                        coefficient=stod(val);
                    }
                    else if (key.compare("MOI")==0){
                        moi=stod(val);
                    }
                    else if (key.compare("left_element")==0){
                        left_element_name=val;
                    }
                    else if ((key.compare("right_element")==0)||(key.compare("object")==0)){
                        //going to assume that forces act from the left (in either direction)
                        right_element_name=val;
                    }
                    else if (key.compare("!verbose")==0){
                        if (val.compare("true")==0){
                            verbosity_=true;
                        }
                        else if (val.compare("false")==0){
                            verbosity_=false;
                        }
                    }
                }

            }
            input_file.close();
            //make last element in file, unless it's the ground object which should have already been made if it exists in the input file

            if (existing_element==true){
                std::cout << "Modifying " << element_type<< " with name " << element_name << "\n";

                //modifying existing element (likely adding another element that it's connected to on its left or right)
                int relevant_index=find_element_index(element_name);
                MechanicalCircuitElement tmp_element=circuit_element_list_.at(relevant_index);
                if (std::find(tmp_element.left_circuit_element_names_.begin(),tmp_element.left_circuit_element_names_.end(),left_element_name)==tmp_element.left_circuit_element_names_.end()){
                    //element name isn't already in that list
                    tmp_element.left_circuit_element_names_.push_back(left_element_name);
                }
                if (std::find(tmp_element.right_circuit_element_names_.begin(),tmp_element.right_circuit_element_names_.end(),right_element_name)==tmp_element.right_circuit_element_names_.end()){
                    //element name isn't already in that list
                    tmp_element.right_circuit_element_names_.push_back(right_element_name);
                }
            }
            else {
                std::cout << "Adding " << element_type<< " with name " << element_name << "\n";
                add_circuit_element(element_name,element_type,coefficient,moi,left_element_name,right_element_name);
                list_of_element_names.push_back(element_name);
            }

        }

    
    void add_circuit_element(std::string element_name,std::string element_type,double coefficient,double moi=0,std::string left_circuit_element_name="",std::string right_circuit_element_name=""){
        MechanicalCircuitElement new_element(element_name,element_type,coefficient,moi,left_circuit_element_name,right_circuit_element_name);
        circuit_element_list_.push_back(new_element);
    }

    int find_element_index(std::string element_name){
        auto lambdafunc = [element_name](MechanicalCircuitElement& tmp_element) {
            return (tmp_element.name_.compare(element_name)==0);
        };
        auto iterator = std::find_if(circuit_element_list_.begin(),circuit_element_list_.end(),lambdafunc);
        if (iterator != circuit_element_list_.end()){
            return std::distance(circuit_element_list_.begin(),iterator);
        }
        else {
            std::cout << "didn't find input element name " << element_name << "in mechanical circuit element list\n";
            return -1;
        }
    }
};





class ElectricalCircuitElement
{
    public:
        std::string name_="";
        std::string type_="";
        std::vector<std::string> left_circuit_element_names_; 
        std::vector<std::string> right_circuit_element_names_;
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
        bool verbosity_=false;
    public:
        std::vector<std::string> output_string_list_={};

        ElectricalCircuit(MechanicalCircuit input_mechanical_circuit,std::string analogy){
            verbosity_=input_mechanical_circuit.verbosity_;
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
        if (std::find(right_element.left_circuit_element_names_.begin(),right_element.left_circuit_element_names_.end(),left_element.name_)==right_element.left_circuit_element_names_.end()){
            right_element.left_circuit_element_names_.push_back(left_element.name_);
            circuit_element_list_.at(right_elem_index)=right_element;
        }
        if (std::find(left_element.right_circuit_element_names_.begin(),left_element.right_circuit_element_names_.end(),right_element.name_)==left_element.right_circuit_element_names_.end()){
            left_element.right_circuit_element_names_.push_back(right_element.name_);
            circuit_element_list_.at(left_elem_index)=left_element;
        }
                
        

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
            std::cout << "didn't find input element name " << element_name << " in circuit element list\n";
            return -1;
        }
    }


    void force_current_conversion(MechanicalCircuit input_mechanical_circuit){
        //this version is using the force-current analogy
        int force_index=0;
        //first, convert masses to grounded capacitors
        std::cout << "converting to electrical circuit\n";
        for (size_t element_index=0;element_index<input_mechanical_circuit.circuit_element_list_.size();element_index++){
            MechanicalCircuitElement mech_element=input_mechanical_circuit.circuit_element_list_.at(element_index);

            if (mech_element.type_.compare("mass")==0){
                if (mech_element.name_.compare("ground")!=0){
                    add_circuit_element(mech_element.name_+"_ground","ground",0);
                }
                add_circuit_element(mech_element.name_,"capacitor",mech_element.coefficient_);
            }
            //next, convert dampers to resistors and add the elements they are connected to (if they don't exist yet)
            else if (mech_element.type_.compare("damper")==0){
                add_circuit_element(mech_element.name_,"resistor",1/mech_element.coefficient_);
            }
            //next convert springs to inductors
            else if (mech_element.type_.compare("spring")==0){
                add_circuit_element(mech_element.name_,"inductor",1/mech_element.coefficient_);
            }
            else if (mech_element.type_.compare("force")==0){
                add_circuit_element(mech_element.name_+"_ground","ground",0);
                add_circuit_element(mech_element.name_,"current_source",mech_element.coefficient_);
                if (force_index==0){
                    first_force_name_=mech_element.name_;
                }
                force_index++;
            }
        }
        //all elements should be added now
        //now we need to connect everything (populating left_circuit_element_names and right_circuit_element_names vecs)
        for (size_t element_index=0; element_index<input_mechanical_circuit.circuit_element_list_.size();element_index++){
            MechanicalCircuitElement mech_element=input_mechanical_circuit.circuit_element_list_.at(element_index);

            if ((mech_element.type_.compare("mass")==0)&&(mech_element.name_.compare("ground")!=0)){
                connect_elements(mech_element.name_,mech_element.name_+"_ground");
            }
            else if (mech_element.type_.compare("force")==0){
                connect_elements(mech_element.name_+"_ground",mech_element.name_);
                connect_elements(mech_element.name_,mech_element.right_circuit_element_names_.at(0));
            }

            else if ((mech_element.type_.compare("damper")==0)||(mech_element.type_.compare("spring")==0)){
                for (int lhs_ind=0;lhs_ind<mech_element.left_circuit_element_names_.size();lhs_ind++){
                    connect_elements(mech_element.left_circuit_element_names_.at(lhs_ind),mech_element.name_);
                }
                for (int rhs_ind=0;rhs_ind<mech_element.right_circuit_element_names_.size();rhs_ind++){
                    connect_elements(mech_element.name_,mech_element.right_circuit_element_names_.at(rhs_ind));
                }
                
            }
        }

    }


    void visit_element(ElectricalCircuitElement current_elem,std::vector<std::vector<std::string>>& input_circuit_grid, std::vector<std::string> input_circuit_grid_row, int input_row_index, int input_column_index,std::vector<std::string>& input_visited_elements,int input_padded_string_length){
        if (verbosity_==true){
            std::cout << "starting search at element " << current_elem.name_ << "\n";
            std::cout << "adding " << current_elem.name_ << " to row index " << input_row_index << " and column index " << input_column_index << "\n";
        }

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

        if (current_elem.left_circuit_element_names_.size()>0){
            if (verbosity_==true){
                std::cout << "found " << current_elem.left_circuit_element_names_.size() <<  " element(s) to the left, exploring that way first\n";
            }
            for (int lhs_index=0;lhs_index<current_elem.left_circuit_element_names_.size();lhs_index++){
                ElectricalCircuitElement new_elem=circuit_element_list_.at(find_element_index(current_elem.left_circuit_element_names_.at(lhs_index)));
                if (std::find(input_visited_elements.begin(),input_visited_elements.end(),new_elem.name_)!=input_visited_elements.end()){
                    //if that element has already been explored, skip it
                    if (verbosity_==true){
                        std::cout << "already processed that one, skipping it\n";
                    }
                    continue;
                }
                if ((lhs_index==0)&&(current_elem.left_circuit_element_names_.size()==0)){
                    if (new_elem.name_.compare(input_circuit_grid.at(input_row_index).at(input_column_index-1))!=0){
                        std::cout << "Huh?";
                    }
                }
                else {

                    if (lhs_index>0){
                        if (verbosity_==true){
                            std::cout << "couldn't find an element name of " << new_elem.name_ << " in input grid, so adding a row\n";
                        }
                        input_circuit_grid.push_back(input_circuit_grid_row);
                        if (verbosity_==true){
                            std::cout << "adding a branch collapse indicator at row " << input_row_index+1 << " and column index " << input_column_index << "\n";
                        }
                        

                        input_circuit_grid.at(input_row_index+1).at(input_column_index)="b.c.u.i.t.c.";//branch collapsing up into this column
                        input_circuit_grid.push_back(input_circuit_grid_row);

                    }
                    visit_element(new_elem,input_circuit_grid,input_circuit_grid_row,input_row_index+(2*lhs_index),input_column_index-1,input_visited_elements,input_padded_string_length);

                }
            }

        }

        //now explore right
        if (current_elem.right_circuit_element_names_.size()>0){
            if (verbosity_==true){
                std::cout << "found " << current_elem.right_circuit_element_names_.size() <<  " element(s) to the right, exploring that way\n";
            }
            
            int effective_index=0;
            for (int rhs_index=0;rhs_index<current_elem.right_circuit_element_names_.size();rhs_index++){
                ElectricalCircuitElement new_elem=circuit_element_list_.at(find_element_index(current_elem.right_circuit_element_names_.at(rhs_index)));
                if (verbosity_==true){
                    std::cout << "found an elem on the right called " << new_elem.name_ << "\n";
                }
                
                if (std::find(input_visited_elements.begin(),input_visited_elements.end(),new_elem.name_)!=input_visited_elements.end()){
                    //if that element has already been explored, skip it
                    if (verbosity_==true){
                        std::cout << "already processed that one, skipping it\n";
                    }
                    continue;
                }

                if ((rhs_index==0)&&(input_circuit_grid.at(input_row_index).at(input_column_index+1).size()>0)){
                    if (verbosity_==true){
                        std::cout << "Already an element there, iterating effective index to avoid overwriting\n";
                    }
                    
                    effective_index++;
                }
                if (effective_index>0){
                    if (verbosity_==true){
                        std::cout << "couldn't find an element name of " << new_elem.name_ << " in input grid, so adding a row\n";
                    }
                    
                    input_circuit_grid.push_back(input_circuit_grid_row);
                    if (verbosity_==true){
                        std::cout << "adding a branch expansion indicator at row " << input_row_index+1 << " and column index " << input_column_index << "\n";
                    }
                    

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
        std::cout << "First force name: " << first_force_name_ << "\n";
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
            output_string_list_.push_back(row_str+"\n");

        }

        output_string_list_.push_back("\nLegend:\n");
        output_string_list_.push_back("[G]: Ground\n");
        output_string_list_.push_back("[C](x): Capacitor with capacitance x\n");
        output_string_list_.push_back("[R](x): Resistor with resistance x\n");
        output_string_list_.push_back("[I](x): Inductor with inductance x\n");
        output_string_list_.push_back("[->](x): Current source with current x\n");

        for (int str_ind=0;str_ind<output_string_list_.size();str_ind++){
            std::cout << output_string_list_.at(str_ind);
        }
    }
};
