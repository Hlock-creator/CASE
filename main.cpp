#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>

struct Variable {
    std::string name;
    std::string value;
};

struct ClassField{
    std::string nameCls;
    std::string nameVar;
    std::string value;
};

std::string trim(const std::string& str){
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
}


std::string get_value(const std::string& token, const std::vector<Variable>& vars){
    if (token.length() >= 2 && token.front() == '"' && token.back() == '"' ){
        return token.substr(1, token.length() - 2);
    }
    for (const auto& var: vars){
        if (var.name == token){
            return var.value;
        }
    }
    return token;
};


bool check_one(std::string cond, const std::vector<Variable>& vars){
    size_t neq_pos = cond.find("!=");
    if (neq_pos != std::string::npos){
        std::string left = trim(cond.substr(0, neq_pos));
        std::string right = trim (cond.substr(neq_pos + 2));
        return get_value(left, vars) != get_value(right, vars);
    }

    size_t op_pos = cond.find_first_of("=<>");
    if (op_pos == std::string::npos) return false;

    std::string left = trim(cond.substr(0, op_pos));
    std::string right = trim(cond.substr(op_pos + 1));
    std::string l_val = get_value(left, vars);
    std::string r_val = get_value(right, vars);

    if (cond[op_pos] == '=') return l_val == r_val;

    try{
        int l_num = std::stoi(l_val);
        int r_num = std::stoi(r_val);
        if(cond[op_pos] == '>') return l_num > r_num;
        if (cond[op_pos] == '<') return l_num < r_num;
    } catch (...){
        return false;
    }
    return false;
}

int main(int argc, char* argv[]) {

    if (argc < 2){
        std::cout << "error: no input file Specified\n";
        std::cout << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::string ext = ".cspp";

    if (filename.length() <= ext.length() || filename.substr(filename.length() - ext.length()) != ext){
        std::cout << "error: invalid file extension (expected " << ext << ")" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cout << "error: cannot open file " << argv[1] << std::endl;
        return 1;
    }

    std::vector<std::string> lines;
    std::string temp_line;
    while (std::getline(file, temp_line)){
        lines.push_back(temp_line);
    }
    file.close();

    std::string stroka;
    std::string function;
    std::vector<Variable> vars;

    std::vector<ClassField> class_templates;
    bool inside_class = false;
    std::string current_class = "";

    bool execution_flag = true;
    bool inside_if = false;
    bool condition_result = true;
    int math_result = true;

    int while_start = -1;
    bool skip_while = false;



    for (size_t i = 0; i < lines.size(); i++) {
        std::string stroka = trim(lines[i]);
        stroka = trim(stroka);
        if (stroka.empty()) continue;

        if (inside_class){
            if (stroka == "}"){
                inside_class = false;
                current_class = "";
                continue;
        }
        else if (stroka.rfind("set ", 0) == 0){
            std::stringstream ss(stroka.substr(4));
            std::string name;
            std::string equals;
            std::string value;
            ss >> name >> equals;
            std::getline(ss >> std::ws, value);
            value = trim(value);

            if (name.empty() || equals != "=" || value.empty()){
                std::cout << "error: bad set syntax in class" << std::endl;
                continue;
            }

            class_templates.push_back({current_class,name,value});
            continue;
        }
        else{
            std::cout << "error: only 'set' allowed inside class" << std::endl;
            continue;
            }
        }

        if (stroka == "}"){
            if (skip_while){
                skip_while = false;
                continue;
            }
            if (while_start != -1){
                i = while_start - 1;
                continue;
            }
        }

        if (skip_while){
            continue;
        }
        

        size_t first_non_space = stroka.find_first_not_of(" \t");
        if (first_non_space != std::string::npos) {
            stroka = stroka.substr(first_non_space);
        } else {
            stroka.clear();
        }

        if (stroka.empty()) {
            continue;
        }

        // IF ELSE IF ELSE

        if (stroka == "endif") {
            inside_if = false;
            execution_flag = true;
            continue;
        }

        else if (stroka == "else") {
            if (inside_if) {
                execution_flag = !condition_result;
            }
            continue;
        }

        else if (stroka.rfind("if ", 0) == 0) {
            inside_if = true;
            std::string condition = stroka.substr(3);

            size_t and_pos = condition.find('&');
            size_t or_pos = condition.find('|');

            if (and_pos != std::string::npos){
                std::string left_part = trim(condition.substr(0, and_pos));
                std::string right_part = trim(condition.substr(and_pos + 1));
                condition_result = check_one(left_part, vars) && check_one(right_part, vars);
            }
            else if (or_pos != std::string::npos){
                std::string left_part = trim(condition.substr(0, or_pos));
                std::string right_part = trim(condition.substr(or_pos + 1));
                condition_result = check_one(left_part, vars) || check_one(right_part, vars);
            }
            else {
                condition_result = check_one(condition, vars);
            }

            execution_flag = condition_result;
            continue;
        }

        if (!execution_flag){
            continue;
        }


        // ОБЫЧНЫЙ PRINT
        else if (stroka.rfind("print ", 0) == 0) {
        std::string expr = trim(stroka.substr(6));
        std::string final_output = "";
        bool in_quotes = false;
        std::string current_part = "";

            
        for (size_t k = 0; k <= expr.length(); k++) {
            if (k < expr.length() && expr[k] == '"') {
                in_quotes = !in_quotes;
                current_part += expr[k];
            }
            else if (k == expr.length() || (expr[k] == '+' && !in_quotes)) {
                current_part = trim(current_part);

                if (current_part.length() >= 2 && current_part.front() == '"' && current_part.back() == '"') { 
                    final_output += current_part.substr(1, current_part.length() - 2);
                } else if (!current_part.empty()) {
                    bool found = false;
                    for (const auto& v : vars) {
                        if (v.name == current_part) {
                            final_output += v.value;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        std::cout << "error: undefined variable '" << current_part << "'" << std::endl;
                        final_output = "";
                        break;
                    }
                }
                current_part = "";
            }
            else {
                current_part += expr[k];
            }
        }

        if (!final_output.empty()) {
            std::cout << final_output << std::endl;
        }
    }


        // УСТАНОВКА ПЕРЕМЕННОЙ
        else if (stroka.rfind("set ", 0) == 0) {
            std::stringstream ss(stroka.substr(4));
            std::string name;
            std::string equals;
            std::string value;

            ss >> name;
            ss >> equals;
            std::getline(ss >> std::ws, value);
            value = trim(value);

            if (name.empty() || equals != "=" || value.empty()) {
                std::cout << "error: bad set syntax" << std::endl;
                continue;
            }

            int found_index = -1;
            for (size_t i = 0; i < vars.size(); i++) {
                if (vars[i].name == name) {
                    found_index = static_cast<int>(i);
                    break;
                }
            }

            if (found_index != -1) {
                vars[found_index].value = value;
            } else {
                vars.push_back({name, value});
            }
        }

        // СТРОКА ВВОД СТРОКИ
        else if (stroka.rfind ("ent ", 0) == 0){    
            std::string nameStr = trim(stroka.substr(4));

            if (!nameStr.empty() && nameStr.back() == ' ') nameStr.pop_back();
            if (!nameStr.empty() && nameStr.front() == ' ') nameStr = nameStr.substr(1);
            
            if (nameStr.empty()){
                std::cout << "error: bad ent name" << std::endl;
                continue;
            }

            std::string user_input;
            std::getline(std::cin, user_input);

            int found_index = -1;
            for (size_t i = 0; i < vars.size(); i++){
                if (vars[i].name == nameStr){
                    found_index = static_cast<int>(i);
                    break;
                }
            }

            if (found_index != -1){
                vars[found_index].value = user_input;
            }else {
                vars.push_back({nameStr, user_input});
            }
        }

        // МАТЕМАТИКА

        else if (stroka.rfind("math ", 0) == 0) {
            std::string mathStr = stroka.substr(5);
            
            size_t eq_pos = mathStr.find('=');
            if (eq_pos == std::string::npos) {
                std::cout << "error: bad math syntax (missing '=')" << std::endl;
                continue;
            }

            std::string res_var = trim(mathStr.substr(0, eq_pos));
            std::string expression = trim(mathStr.substr(eq_pos + 1));

            if (!res_var.empty() && res_var.back() == ' ') res_var.pop_back();
            if (!res_var.empty() && res_var.front() == ' ') res_var = res_var.substr(1);

            size_t zn_pos = expression.find_first_of("+-*/");

            if (zn_pos != std::string::npos) {
                std::string left = trim(expression.substr(0, zn_pos));
                std::string right = trim(expression.substr(zn_pos + 1));

                if (!left.empty() && left.back() == ' ') left.pop_back();
                if (!left.empty() && left.front() == ' ') left = left.substr(1);
                if (!right.empty() && right.front() == ' ') right = right.substr(1);
                if (!right.empty() && right.back() == ' ') right.pop_back();

                std::string left_str = get_value(left, vars);
                std::string right_str = get_value(right, vars);

                int left_val = 0, right_val = 0;
                try {
                    left_val = std::stoi(left_str);
                    right_val = std::stoi(right_str);
                } catch (...) {
                    std::cout << "error: non-numeric operands in math" << std::endl;
                    continue;
                }

                if (expression[zn_pos] == '+') {
                    math_result = left_val + right_val;
                }
                else if (expression[zn_pos] == '-') {
                    math_result = left_val - right_val;
                }
                else if (expression[zn_pos] == '*') {
                    math_result = left_val * right_val;
                }
                else if (expression[zn_pos] == '/') {
                    if (right_val != 0) {
                        math_result = left_val / right_val;
                    } else {
                        std::cerr << "error: Division by zero!" << std::endl;
                        math_result = 0;
                    }
                }

                int found_index = -1;
                for (size_t i = 0; i < vars.size(); i++) {
                    if (vars[i].name == res_var) {
                        found_index = static_cast<int>(i);
                        break;
                    }
                }

                std::string res_str = std::to_string(math_result);
                if (found_index != -1) {
                    vars[found_index].value = res_str;
                } else {
                    vars.push_back({res_var, res_str});
                }

            } else {
                std::cout << "error: no math operator found" << std::endl;
            }
        }


        // ООП 

        else if (stroka.rfind("Class ", 0) == 0) {
            std::string classStr = stroka.substr(6);
            std::stringstream ss (classStr);
            std::string nameCls;
            std::string brace;
            ss >> nameCls >> brace;

                  
            if (nameCls.empty() || brace != "{") {
                std::cout << "error: character missing '{'" << std::endl;
                continue;
            }else{
                current_class = nameCls;
                inside_class = true;
                continue;
            }
        }

        else if (stroka.rfind("new ", 0) == 0){
            std::string newCls = stroka.substr(4);
            std::stringstream ss (newCls);
            std::string className;
            std::string objName;
            ss >> className >> objName;

            if (className.empty() || objName.empty()){
                std::cout << "error: bad new syntax" << std::endl;
                continue; 
            }

            bool found_class = false;
            for (const auto& field : class_templates){
                if (field.nameCls == className){
                   found_class = true;
                   std::string full_var_name = objName + "." + field.nameVar;
                   vars.push_back({full_var_name, field.value});
                }
            }

            if (!found_class){
                std::cout << "error: class not found" << std::endl;
            }
        }



        // WHILE

        else if (stroka.rfind("while ", 0 ) == 0){
            std::string whileStr = trim(stroka.substr(6));
            size_t brace_pos = whileStr.find('{');

            if (brace_pos == std::string::npos || brace_pos != whileStr.length() - 1){
                std::cout << "error: bad while syntax" << std::endl;
                continue;
            }

            std::string condition = trim(whileStr.substr(0, brace_pos));
            if (condition.empty()){
                std::cout << "error: empty condition in while" << std::endl;
                continue;
            }

            bool while_cond = check_one(condition, vars);

            if (while_cond){
                while_start = static_cast<int>(i);
            } else{
                while_start = -1;
                skip_while = true;
            }
            continue;
        }



        // ЧИТАНИЕ, ЗАПИСЬ И ПРОЧИЕ РАБОТЫ С ФАЙЛАМИ

        else if (stroka.rfind("read ", 0) == 0){
            std::string fileStr = trim(stroka.substr(5));
            std::stringstream ss (fileStr);
            std::string filePut;
            std::string setName;
            ss >> filePut >> setName;

            if (filePut.empty() || setName.empty()){
                std::cout << "error: file or variable name not specified" << std::endl;
                continue;
            }

            std::string filepath = get_value(filePut, vars);

            std::ifstream in_file(filepath);
            if (!in_file.is_open()){
                std::cout << "error: cannot read file '" << filePut << "'" << std::endl;
                continue;
            }

            std::ostringstream buf;
            buf << in_file.rdbuf();
            std::string content = buf.str();
            in_file.close();

            int found_index = -1;
            for (size_t k = 0; k < vars.size(); k++){
                if (vars[k].name == setName){
                    found_index = static_cast<int>(k);
                    break;
                }
            }

            if (!content.empty() && content.back() == '\n') content.pop_back();

            if (found_index != -1){
                vars[found_index].value = content;
            }else {
                vars.push_back({setName, content});
            }
        }


        else if (stroka.rfind("write ", 0) == 0) {
        std::string fileStr = trim(stroka.substr(6));
        std::stringstream ss(fileStr);
        std::string filePut;
        std::string rawData;

        ss >> filePut;
        std::getline(ss >> std::ws, rawData);
        rawData = trim(rawData);

        if (filePut.empty() || rawData.empty()) {
            std::cout << "error: bad write syntax (use: write \"file.txt\" data)" << std::endl;
            continue;
        }

        std::string filepath = get_value(filePut, vars);
        std::string data = get_value(rawData, vars);

        std::ofstream out_file(filepath);
        if (!out_file.is_open()) {
            std::cout << "error: cannot write to file '" << filepath << "'" << std::endl;
            continue;
        }

        out_file << data << std::endl;
        out_file.close();

    }



      else if (stroka.rfind("delete ", 0) == 0){
        std::string filePut = trim(stroka.substr(7));

        if (filePut.empty()){
            std::cout << "error: file name not specified for delete" << std::endl;
            continue;
        }

        std::string filepath = get_value(filePut, vars);

        if (!std::filesystem::remove(filepath)){
            std::cout << "error: cannot delete file '" << filepath << "'" << std::endl;
        }
    }

        // НЕИЗВЕСТНАЯ КОМАНДА
        else {
            std::cout << "error: unknown command" << std::endl;
        }
    }
    return 0;
}