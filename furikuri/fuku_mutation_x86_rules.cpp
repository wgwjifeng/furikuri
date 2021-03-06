#include "stdafx.h"
#include "fuku_mutation_x86_rules.h"



bool fukutate_jcc(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {
    
    auto next_line = lines_iter; next_line++;

    if (next_line != code_holder.get_lines().begin()) {

        const uint8_t* code = &lines_iter->get_op_code()[lines_iter->get_op_pref_size()];

        uint8_t cond;

        if (code[0] == 0x0F) {
            cond = code[1] & 0xF;
        }
        else {
            cond = code[0] & 0xF;
        }

        fuku_instruction line[2];

        line[0] = f_asm.jcc(fuku_condition(cond ^ 1), 0)
            .set_custom_flags(lines_iter->get_custom_flags())
            .set_rip_relocation_idx(code_holder.create_rip_relocation(2, &(*next_line)))
            .set_instruction_flags(fuku_instruction_full_mutated);

        line[1] = f_asm.jmp(0)
            .set_custom_flags(lines_iter->get_custom_flags())
            .set_rip_relocation_idx(lines_iter->get_rip_relocation_idx())
            .set_label_idx(lines_iter->get_label_idx())
            .set_source_virtual_address(lines_iter->get_source_virtual_address());

        code_holder.get_rip_relocations()[line[0].get_rip_relocation_idx()].offset = 2;
        code_holder.get_rip_relocations()[line[1].get_rip_relocation_idx()].offset = 1;

        code_holder.get_lines().insert(lines_iter, line[0]);
        *lines_iter = line[1];

        return true;
    }

    return false;
}

bool fukutate_jmp(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {
  

    if (lines_iter->get_op_code()[0] == 0xE9 && !IS_HAS_FULL_BITES(lines_iter->get_instruction_flags(), fuku_instruction_bad_stack_pointer)) {

        if (FUKU_GET_RAND(0, 1)) {
            //push jmpdst
            //ret

            fuku_instruction line[2];

            line[0] = f_asm.push_imm32(0)
                .set_custom_flags(lines_iter->get_custom_flags())
                .set_relocation_first_idx(code_holder.create_relocation_lb(1, code_holder.get_rip_relocations()[lines_iter->get_rip_relocation_idx()].label_idx, 0));

            line[1] = f_asm.ret(0)
                .set_custom_flags(lines_iter->get_custom_flags())
                .set_label_idx(lines_iter->get_label_idx())
                .set_source_virtual_address(lines_iter->get_source_virtual_address());

            code_holder.delete_rip_relocation(lines_iter->get_rip_relocation_idx());


            code_holder.get_lines().insert(lines_iter, line[0]);
            *lines_iter = line[1];
        }
        else {
            //je(jcc) jmpdst
            //jne(jcc) jmpdst

            fuku_instruction line[2];
            uint8_t cond = FUKU_GET_RAND(0, 15);

            line[0] = f_asm.jcc(fuku_condition(cond), 0)
                .set_custom_flags(lines_iter->get_custom_flags())
                .set_rip_relocation_idx(code_holder.create_rip_relocation(code_holder.get_rip_relocations()[lines_iter->get_rip_relocation_idx()]));

            line[1] = f_asm.jcc(fuku_condition(cond^1), 0)
                //.set_custom_flags(lines_iter->get_custom_flags()) //else can be changed flags for jcc
                .set_label_idx(lines_iter->get_label_idx())
                .set_rip_relocation_idx(lines_iter->get_rip_relocation_idx())
                .set_source_virtual_address(lines_iter->get_source_virtual_address());

            code_holder.get_rip_relocations()[line[0].get_rip_relocation_idx()].offset = 2;
            code_holder.get_rip_relocations()[line[1].get_rip_relocation_idx()].offset = 2;

            code_holder.get_lines().insert(lines_iter, line[0]);
            *lines_iter = line[1];
        }


        return true;
    }
  
    return false;
}

bool fukutate_ret(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    
    if (lines_iter->get_op_code()[0] == 0xC3) { //ret
        //lea esp,[esp + 4]
        //jmp [esp - 4]

        fuku_instruction line[2];

        line[0] = f_asm.lea(fuku_reg86::r_ESP, fuku_operand86(fuku_reg86::r_ESP, 4))
            .set_custom_flags(lines_iter->get_custom_flags());

        line[1] = f_asm.jmp(fuku_operand86(r_ESP, -4))
            .set_instruction_flags(fuku_instruction_bad_stack_pointer)
            .set_custom_flags(lines_iter->get_custom_flags())
            .set_label_idx(lines_iter->get_label_idx())
            .set_source_virtual_address(lines_iter->get_source_virtual_address());
        

        code_holder.get_lines().insert(lines_iter, line[0]);
        *lines_iter = line[1];

        return true;

    }
    else if (lines_iter->get_op_code()[0] == 0xC2) { //ret 0x0000
        //lea esp,[esp + (4 + stack_offset)]
        //jmp [esp - 4 - stack_offset]

        uint16_t ret_stack = *(uint16_t*)&lines_iter->get_op_code()[1];

        fuku_instruction line[2];

        line[0] = f_asm.lea(fuku_reg86::r_ESP, fuku_operand86(fuku_reg86::r_ESP, 4 + ret_stack))
            .set_custom_flags(lines_iter->get_custom_flags());

        line[1] = f_asm.jmp(fuku_operand86(r_ESP, -4 - ret_stack))
            .set_instruction_flags(fuku_instruction_bad_stack_pointer)
            .set_custom_flags(lines_iter->get_custom_flags())
            .set_label_idx(lines_iter->get_label_idx())
            .set_source_virtual_address(lines_iter->get_source_virtual_address());


        
        code_holder.get_lines().insert(lines_iter, line[0]);
        *lines_iter = line[1];

        return true;
    }
    

    return false;
}

bool fukutate_add(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    /*
    auto& target_line = lines[current_line_idx];
    
    if (((target_line.get_flags() & fuku_instruction_bad_stack) == 0) &&
        (target_line.get_modified_flags() & target_line.get_useless_flags()) == target_line.get_modified_flags() &&
        !target_line.get_relocation_f_imm_offset()) {

        const uint8_t* code = &target_line.get_op_code()[target_line.get_op_pref_size()];

        if (code[0] == 0x05 ||  //add reg,imm
            ((code[0] == 0x81 || code[0] == 0x83) && (code[1] >= 0xC0 && code[1] < 0xC8)) ) {

            fuku_reg86 reg1;
            uint32_t val;

            if (code[0] == 0x05) {
                reg1 = fuku_reg86::r_EAX;
                val = *(uint32_t*)&code[1];
            }
            else {
                reg1 = fuku_reg86(code[1] - 0xC0);
                if (code[0] == 0x81) {
                    val = *(uint32_t*)&code[2];
                }
                else {
                    val = *(int8_t*)&code[2];
                }
            }

            if (reg1 == fuku_reg86::r_ESP) { return false; }

            switch (FUKU_GET_RAND(1, 2)) {
            case 1: {
                unsigned int passes_number = FUKU_GET_RAND(2, 4);
                uint32_t current_val = 0;

                for (unsigned int pass = 0; pass < passes_number; pass++ ) {

                    switch (FUKU_GET_RAND(1, 2)) {

                    case 1: {
                        if (pass + 1 < passes_number) {
                            uint32_t rand_val = FUKU_GET_RAND(0, 0xFFFFFFFF);
                            out_lines.push_back(f_asm.sub(reg1, fuku_immediate86(rand_val)).set_useless_flags(target_line.get_useless_flags()));
                            current_val -= rand_val;
                        }
                        else {
                            out_lines.push_back(f_asm.sub(reg1, fuku_immediate86(current_val - val)).set_useless_flags(target_line.get_useless_flags()));
                            current_val -= (current_val - val);
                        }
                        break;
                    }
                    case 2: {
                        if (pass + 1 < passes_number) {
                            uint32_t rand_val = FUKU_GET_RAND(0, 0xFFFFFFFF);
                            out_lines.push_back(f_asm.add(reg1, fuku_immediate86(rand_val)).set_useless_flags(target_line.get_useless_flags()));
                            current_val += rand_val;
                        }
                        else {
                            out_lines.push_back(f_asm.add(reg1, fuku_immediate86(val - current_val)).set_useless_flags(target_line.get_useless_flags()));
                            current_val += (val - current_val);                       
                        }
                        break;
                    }
                    }
                }
                break;
            }

            case 2: {
                out_lines.push_back(f_asm.sub(reg1, fuku_immediate86((-(int32_t)val))).set_useless_flags(target_line.get_useless_flags()));
                break;
            }
            }
            

            return true;
        }
    }
    */
    return false;
}

bool fukutate_sub(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {
    /*
    auto& target_line = lines[current_line_idx];

    if (((target_line.get_flags() & fuku_instruction_bad_stack) == 0) && 
        (target_line.get_modified_flags() & target_line.get_useless_flags()) == target_line.get_modified_flags() &&
        !target_line.get_relocation_f_imm_offset()) {

        const uint8_t* code = &target_line.get_op_code()[target_line.get_op_pref_size()];

        if (code[0] == 0x2D ||  //sub reg,imm
            ((code[0] == 0x81 || code[0] == 0x83) && (code[1] >= 0xE8 && code[1] < 0xF0))) {

            fuku_reg86 reg1;
            uint32_t val;

            if (code[0] == 0x2D) {
                reg1 = fuku_reg86::r_EAX;
                val = *(uint32_t*)&code[1];
            }
            else {
                reg1 = fuku_reg86(code[1] - 0xE8);
                if (code[0] == 0x81) {
                    val = *(uint32_t*)&code[2];
                }
                else {
                    val = *(int8_t*)&code[2];
                }
            }

            if (reg1 == fuku_reg86::r_ESP) { return false; }

            val = -(int32_t)val;

            switch (FUKU_GET_RAND(1, 2)) {
            case 1: {
                unsigned int passes_number = FUKU_GET_RAND(2, 4);
                uint32_t current_val = 0;

                for (unsigned int pass = 0; pass < passes_number; pass++) {

                    switch (FUKU_GET_RAND(1, 2)) {

                    case 1: {
                        if (pass + 1 < passes_number) {
                            uint32_t rand_val = FUKU_GET_RAND(0, 0xFFFFFFFF);
                            out_lines.push_back(f_asm.sub(reg1, fuku_immediate86(rand_val)).set_useless_flags(target_line.get_useless_flags()));
                            current_val -= rand_val;
                        }
                        else {
                            out_lines.push_back(f_asm.sub(reg1, fuku_immediate86(current_val - val)).set_useless_flags(target_line.get_useless_flags()));
                            current_val -= (current_val - val);
                        }
                        break;
                    }
                    case 2: {
                        if (pass + 1 < passes_number) {
                            uint32_t rand_val = FUKU_GET_RAND(0, 0xFFFFFFFF);
                            out_lines.push_back(f_asm.add(reg1, fuku_immediate86(rand_val)).set_useless_flags(target_line.get_useless_flags()));
                            current_val += rand_val;
                        }
                        else {
                            out_lines.push_back(f_asm.add(reg1, fuku_immediate86(val - current_val)).set_useless_flags(target_line.get_useless_flags()));
                            current_val += (val - current_val);
                        }
                        break;
                    }
                    }
                }
                break;
            }

            case 2: {
                out_lines.push_back(f_asm.sub(reg1, fuku_immediate86((-(int32_t)val))).set_useless_flags(target_line.get_useless_flags()));
                break;
            }
            }


            return true;
        }
    }
    */
    return false;
}

bool fukutate_inc(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {
    /*
    auto& target_line = lines[current_line_idx];

    if (((target_line.get_flags() & fuku_instruction_bad_stack) == 0) && 
        (target_line.get_modified_flags() & target_line.get_useless_flags()) == target_line.get_modified_flags()) {

        const uint8_t* code = &target_line.get_op_code()[target_line.get_op_pref_size()];

        if ((code[0] & 0xF0) == 0x40) { //inc reg_dw 
            fuku_reg86 reg = fuku_reg86(code[0] & 0x0F);
            fuku_instruction l_res;

            if (reg == fuku_reg86::r_ESP) { return false; }

            /*
            (add reg,FFFFFFFF) or (sub reg,1)
            

            if (FUKU_GET_CHANCE(50.f)) {
                l_res = f_asm.add(reg, fuku_immediate86(1));
            }
            else {
                l_res = f_asm.sub(reg, fuku_immediate86(0xFFFFFFFF));
            }

            l_res.set_useless_flags(target_line.get_useless_flags());

            out_lines.push_back(l_res);

            return true;
        }
        
        return false;
    }
    */
    return false;
}

bool fukutate_dec(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {
    /*
    auto& target_line = lines[current_line_idx];

    if ((target_line.get_modified_flags() & target_line.get_useless_flags()) == target_line.get_modified_flags()) {
        const uint8_t* code = &target_line.get_op_code()[target_line.get_op_pref_size()];

        if ((code[0] & 0xF0) == 0x40) { //dec reg_dw
            fuku_reg86 reg = fuku_reg86((code[0] & 0x0F) - 8);
            fuku_instruction l_res;

            if (reg == fuku_reg86::r_ESP) { return false; }

            /*
            (add reg,1) or (sub reg,FFFFFFFF)
            

            if (FUKU_GET_CHANCE(50.f)) {
                l_res = f_asm.add(reg, fuku_immediate86(0xFFFFFFFF));
            }
            else {
                l_res = f_asm.sub(reg, fuku_immediate86(1));
            }

            l_res.set_useless_flags(target_line.get_useless_flags());

            out_lines.push_back(l_res);

            return true;
        }

        return false;
    }
    */
    return false;
}

bool fukutate_cmp(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {
    /*
    auto& target_line = lines[current_line_idx];
    const uint8_t* code = &target_line.get_op_code()[target_line.get_op_pref_size()];

    if (((target_line.get_flags() & fuku_instruction_bad_stack) == 0) &&
        !target_line.get_relocation_f_imm_offset()) {

        if ((code[0] == 0x39 || code[0] == 0x3B) && (code[1] >= 0xC0 && code[1] < 0xC8)) { //cmp reg1,reg2
            fuku_reg86 reg1 = fuku_reg86((code[1] - 0xC0) % 8);
            fuku_reg86 reg2 = fuku_reg86((code[1] - 0xC0) / 8);

            if (code[0] == 0x3B) {
                std::swap(reg1, reg2);
            }


            if (reg1 == fuku_reg86::r_ESP) {
                fuku_reg86 reg3;
                for (reg3 = fuku_reg86::r_EAX; reg3 < fuku_reg86::r_EBX; reg3 = fuku_reg86(reg3 + 1)) {}


                out_lines.push_back(f_asm.push(reg3).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.mov(reg3, reg1).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.lea(reg3, fuku_operand86(reg3, operand_scale::operand_scale_1, 4)).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.sub(reg3, reg2).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.pop(reg3));

                /*
                push reg3
                mov reg3,reg1
                lea reg3, [reg3 + 4]
                sub reg3,reg2
                pop reg3
                
            }
            else {
                out_lines.push_back(f_asm.push(reg1).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.sub(reg1, reg2).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.pop(reg1));

                /*
                push reg1
                sub reg1,reg2
                pop reg1
                
            }

            return true;
        }
        else if (code[0] == 0x39 || (code[0] == 0x81 && code[1] >= 0xF8)) { //cmp reg, imm
            fuku_reg86 reg1;
            uint32_t val;
            if (code[0] == 0x39) {
                reg1 = fuku_reg86::r_EAX;
                val = *(uint32_t*)&code[1];
            }
            else {
                reg1 = fuku_reg86(code[1] - 0xF8);
                val = *(uint32_t*)&code[2];
            }

            if (reg1 == fuku_reg86::r_ESP) {
                fuku_reg86 reg2;
                for (reg2 = fuku_reg86::r_EAX; reg2 < fuku_reg86::r_EBX; reg2 = fuku_reg86(reg2 + 1)) {}

                out_lines.push_back(f_asm.push(reg2).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.mov(reg2, reg1).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.lea(reg2, fuku_operand86(reg2, operand_scale::operand_scale_1, 4)).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.sub(reg2, fuku_immediate86(val)).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.pop(reg2));

                /*
                push reg2
                mov reg2,reg1
                lea reg2, [reg2 + 4]
                sub reg2,imm
                pop reg3
                
            }
            else {
                out_lines.push_back(f_asm.push(reg1).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.sub(reg1, fuku_immediate86(val)).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.pop(reg1));

                /*
                push reg
                sub reg, imm
                pop reg
                
            }


            return true;
        }
    }
    */
    return false;
}

bool fukutate_and(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    //A and B = (A or B) xor A xor B
    /*
    auto& target_line = lines[current_line_idx];

    if (((target_line.get_flags() & fuku_instruction_bad_stack) == 0) && 
        (target_line.get_modified_flags() & target_line.get_useless_flags()) == target_line.get_modified_flags() &&
        !target_line.get_relocation_f_imm_offset()) {

        const uint8_t* code = &target_line.get_op_code()[target_line.get_op_pref_size()];

        if (
            (code[0] == 0x21 || code[0] == 0x23) && code[1] >= 0xC0) { //and reg_dw, reg_dw
            fuku_reg86 reg1 = fuku_reg86( (code[1] - 0xC0) % 8);
            fuku_reg86 reg2 = fuku_reg86( (code[1] - 0xC0) / 8);
            fuku_reg86 reg3 = fuku_reg86::r_EAX;

            if (code[0] == 0x23) {  std::swap(reg1, reg2); }

            for (reg3 = fuku_reg86::r_EAX; reg3 < fuku_reg86::r_EBX; reg3 = fuku_reg86(reg3+1)) {}

            out_lines.push_back(f_asm.push(reg3).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.mov(reg3, reg1).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.or(reg1,reg2).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.xor(reg1, reg3).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.xor(reg1, reg2).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.pop(reg3).set_useless_flags(target_line.get_useless_flags()));

            /*
            push reg3
            mov reg3, reg1
            or  reg1, reg2
            xor reg1, reg3
            xor reg1, reg2
            pop reg3
            

            return true;
        }
        else if (( (code[0] == 0x81 || code[0] == 0x83) && (code[1] & 0xF0) == 0xE0 && code[1] < 0xE8) || code[1] == 0x25) { //and reg_dw , val //and reg_b , val
            fuku_reg86 reg1;
            fuku_reg86 reg2;
            uint32_t val;

            if (code[1] == 0x25) {
                reg1 = fuku_reg86::r_EAX;
                reg2 = fuku_reg86::r_ECX;
                val = *(uint32_t*)&code[1];
            }
            else {
                reg1 = fuku_reg86((code[1] - 0xE0) & 0x0F);
                reg2 = fuku_reg86::r_ECX;

                if (code[0] == 0x83) {
                    val = *(uint8_t*)&code[2];
                }
                else {
                    val = *(uint32_t*)&code[2];
                }

                for (reg2 = fuku_reg86::r_EAX; reg2 < fuku_reg86::r_EBX; reg2 = fuku_reg86(reg2 + 1)) {}
            }
           
            out_lines.push_back(f_asm.push(reg2).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.mov(reg2, reg1).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.or(reg1, val).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.xor(reg1, reg2).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.xor(reg1, val).set_useless_flags(target_line.get_useless_flags()));
            out_lines.push_back(f_asm.pop(reg2).set_useless_flags(target_line.get_useless_flags()));

            /*
            push reg2
            mov reg2, reg1
            or  reg1, val
            xor reg1, reg2
            xor reg1, val
            pop reg2
            

            return true;
        }

        return false;
    }
    */
    return false;
}

bool fukutate_or(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}

bool fukutate_xor(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}

bool fukutate_test(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {
    /*
    auto& target_line = lines[current_line_idx];
    const uint8_t* code = &target_line.get_op_code()[target_line.get_op_pref_size()];

    if (((target_line.get_flags() & fuku_instruction_bad_stack) == 0) &&
        !target_line.get_relocation_f_imm_offset()) {

        if (code[0] == 0x85 && (code[1] >= 0xC0 && code[1] < 0xC8)) { //test reg1,reg2
            fuku_reg86 reg1 = fuku_reg86((code[1] - 0xC0) % 8);
            fuku_reg86 reg2 = fuku_reg86((code[1] - 0xC0) / 8);

            if (reg1 == fuku_reg86::r_ESP) {
                fuku_reg86 reg3;
                for (reg3 = fuku_reg86::r_EAX; reg3 < fuku_reg86::r_EBX; reg3 = fuku_reg86(reg3 + 1)) {}

                out_lines.push_back(f_asm.push(reg3).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.mov(reg3, reg1).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.lea(reg3, fuku_operand86(reg3, operand_scale::operand_scale_1, 4)).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.and(reg3, reg2).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.pop(reg3));

                /*
                push reg3
                mov reg3,reg1
                lea reg3, [reg3 + 4]
                and reg3,reg2
                pop reg3
                
            }
            else {
                out_lines.push_back(f_asm.push(reg1).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.and(reg1, reg2).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.pop(reg1));
                /*
                push reg1
                and reg1,reg2
                pop reg1
                
            }

            return true;
        }
        else if (code[0] == 0xA9 || (code[0] == 0xF7 && code[1] >= 0xC0 && code[1] < 0xC8)) { //test reg, imm
            fuku_reg86 reg1;
            uint32_t val;
            if (code[0] == 0xA9) {
                reg1 = fuku_reg86::r_EAX;
                val = *(uint32_t*)&code[1];
            }
            else {
                reg1 = fuku_reg86(code[1] - 0xC0);
                val = *(uint32_t*)&code[2];
            }

            if (reg1 == fuku_reg86::r_ESP) {
                fuku_reg86 reg2;
                for (reg2 = fuku_reg86::r_EAX; reg2 < fuku_reg86::r_EBX; reg2 = fuku_reg86(reg2 + 1)) {}

                out_lines.push_back(f_asm.push(reg2).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.mov(reg2, reg1).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.lea(reg2, fuku_operand86(reg2, operand_scale::operand_scale_1, 4)).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.and(reg2, fuku_immediate86(val)).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.pop(reg2));

                /*
                push reg2
                mov reg2,reg1
                lea reg2, [reg2 + 4]
                sub reg2,imm
                pop reg3
                
            }
            else {
                out_lines.push_back(f_asm.push(reg1).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.and(reg1, fuku_immediate86(val)).set_useless_flags(target_line.get_useless_flags()));
                out_lines.push_back(f_asm.pop(reg1));

                /*
                push reg
                and reg, imm
                pop reg
                
            }
            return true;
        }
    }

    */
    return false;
}

bool fukutate_push(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    const uint8_t* code = lines_iter->get_op_code();

    if (code[0] == 0x6A ||  //push imm8/imm32
        code[0] == 0x68 ) {

        //(sub esp,4) or (lea esp,[esp - 4]) 
        //mov [esp],value
        
        uint32_t val;

        if (code[0] == 0x6A) {
            val = *(uint8_t*)&code[1];
        }
        else {
            val = *(uint32_t*)&code[1];
        }

        fuku_instruction line[2];

        uint32_t needed = (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_MODIFY_AF | X86_EFLAGS_MODIFY_CF | X86_EFLAGS_MODIFY_PF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
            line[0] = f_asm.sub(fuku_reg86::r_ESP, fuku_immediate86(4))
                .set_custom_flags(lines_iter->get_custom_flags());
        }
        else {
            line[0] = f_asm.lea(fuku_reg86::r_ESP, fuku_operand86(fuku_reg86::r_ESP, -4))
                .set_custom_flags(lines_iter->get_custom_flags());
        }
               
        line[1] = f_asm.mov(fuku_operand86(fuku_reg86::r_ESP,operand_scale::operand_scale_1),fuku_immediate86(val))
            .set_custom_flags(lines_iter->get_custom_flags())
            .set_instruction_flags(fuku_instruction_bad_stack_pointer)
            .set_label_idx(lines_iter->get_label_idx())
            .set_source_virtual_address(lines_iter->get_source_virtual_address());
        
        if (lines_iter->get_relocation_first_idx() != -1) {
            line[1].set_relocation_first_idx(lines_iter->get_relocation_first_idx());
            code_holder.get_relocations()[lines_iter->get_relocation_first_idx()].offset = 3;            
        }

        code_holder.get_lines().insert(lines_iter, line[0]);
        *lines_iter = line[1];
       
        return true;

    } else if ((code[0] & 0xF0) == 0x50) { //push reg

        //(sub esp,4) or (lea esp,[esp - 4]) 
        //mov [esp],reg

        fuku_reg86 reg = fuku_reg86( code[0] & 0x0F);
        
        fuku_instruction line[2];

        uint32_t needed = (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_MODIFY_AF | X86_EFLAGS_MODIFY_CF | X86_EFLAGS_MODIFY_PF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
            line[0] = f_asm.sub(fuku_reg86::r_ESP, fuku_immediate86(4))
                .set_custom_flags(lines_iter->get_custom_flags());
        }
        else {
            line[0] = f_asm.lea(fuku_reg86::r_ESP, fuku_operand86(fuku_reg86::r_ESP, -4))
                .set_custom_flags(lines_iter->get_custom_flags());
        }


        line[1] = f_asm.mov(fuku_operand86(fuku_reg86::r_ESP, operand_scale::operand_scale_1), reg)
            .set_custom_flags(lines_iter->get_custom_flags())
            .set_instruction_flags(fuku_instruction_bad_stack_pointer)
            .set_label_idx(lines_iter->get_label_idx())
            .set_source_virtual_address(lines_iter->get_source_virtual_address());

        
        code_holder.get_lines().insert(lines_iter, line[0]);
        *lines_iter = line[1];
       
        return true;
    }
   

    return false;
}

bool fukutate_pop(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    
    const uint8_t* code = lines_iter->get_op_code();

     if ((code[0] & 0xF0) == 0x50) {
         //add esp,4
         //mov reg,[esp - 4]
         //     or
         //mov reg,[esp]
         //add esp,4

        fuku_reg86 reg = fuku_reg86(code[0] - 0x58);

        fuku_instruction line[2];

        uint32_t needed = (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_MODIFY_AF | X86_EFLAGS_MODIFY_CF | X86_EFLAGS_MODIFY_PF);

        if (FUKU_GET_RAND(0, 10) < 5) {

            line[0] = f_asm.mov(reg, fuku_operand86(fuku_reg86::r_ESP, operand_scale::operand_scale_1))
                .set_custom_flags(lines_iter->get_custom_flags());


            if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
                line[1] = f_asm.add(fuku_reg86::r_ESP, fuku_immediate86(4))
                    .set_custom_flags(lines_iter->get_custom_flags());
            }
            else {
                line[1] = f_asm.lea(fuku_reg86::r_ESP, fuku_operand86(fuku_reg86::r_ESP, 4))
                    .set_custom_flags(lines_iter->get_custom_flags());
            }
        }
        else {

            if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
                line[0] = f_asm.add(fuku_reg86::r_ESP, fuku_immediate86(4))
                    .set_custom_flags(lines_iter->get_custom_flags());

            }
            else {
                line[0] = f_asm.lea(fuku_reg86::r_ESP, fuku_operand86(fuku_reg86::r_ESP, 4))
                    .set_custom_flags(lines_iter->get_custom_flags());
            }

            line[1] = f_asm.mov(reg, fuku_operand86(fuku_reg86::r_ESP, -4))
                .set_custom_flags(lines_iter->get_custom_flags())
                .set_instruction_flags(fuku_instruction_bad_stack_pointer);      
        }

        line[1].set_label_idx(lines_iter->get_label_idx())
               .set_source_virtual_address(lines_iter->get_source_virtual_address());


        code_holder.get_lines().insert(lines_iter, line[0]);
        *lines_iter = line[1];

        return true;
    }
    
    return false;
}

void generate_junk(fuku_asm_x86& f_asm, fuku_code_holder& code_holder,
    linestorage::iterator lines_iter, uint32_t max_size, size_t junk_size) {


    size_t current_size = 0;

    while (junk_size != current_size) {

        switch (FUKU_GET_RAND(1, min(min(junk_size - current_size, max_size), 7))) {
        case 1: {
            fuku_junk_1b(f_asm, code_holder, lines_iter); current_size += 1;
            break;
        }
        case 2: {
            fuku_junk_2b(f_asm, code_holder, lines_iter); current_size += 2;
            break;
        }
        case 3: {
            fuku_junk_3b(f_asm, code_holder, lines_iter); current_size += 3;
            break;
        }
        case 4: {
            fuku_junk_4b(f_asm, code_holder, lines_iter); current_size += 4;
            break;
        }
        case 5: {
            fuku_junk_5b(f_asm, code_holder, lines_iter); current_size += 5;
            break;
        }
        case 6: {
            fuku_junk_6b(f_asm, code_holder, lines_iter); current_size += 6;
            break;
        }
        case 7: {
            fuku_junk_7b(f_asm, code_holder, lines_iter); current_size += 7;
            break;
        }
        }
    }
}


void fuku_junk_1b(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator lines_iter) {

    //nop
    code_holder.get_lines().insert(lines_iter, f_asm.nop()); 
}

void fuku_junk_2b(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator lines_iter) {

    switch (FUKU_GET_RAND(0, 4)) {

    case 0: { 
        //mov reg1, reg1

        fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EBX));

        code_holder.get_lines().insert(lines_iter, f_asm.mov(reg1, reg1).set_custom_flags(lines_iter->get_custom_flags()));
        break;
    }
    case 1: { 
        //xchg eax, reg2
        //xchg reg2, eax

    jk_2s:

        fuku_reg86 reg1 = fuku_reg86::r_EAX;
        fuku_reg86 reg2 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EDI));

        fuku_instruction line[2];

        if (FUKU_GET_RAND(0, 1)) {
            line[0] = f_asm.xchg(reg1, reg2).set_custom_flags(lines_iter->get_custom_flags());
        }
        else {
            line[0] = f_asm.xchg(reg2, reg1).set_custom_flags(lines_iter->get_custom_flags());
        }

        if (FUKU_GET_RAND(0, 1)) {
            line[1] = f_asm.xchg(reg1, reg2).set_custom_flags(lines_iter->get_custom_flags());
        }
        else {
            line[1] = f_asm.xchg(reg2, reg1).set_custom_flags(lines_iter->get_custom_flags());
        }

        if (reg2 == fuku_reg86::r_ESP) {
            line[1].set_instruction_flags(fuku_instruction_bad_stack_pointer);
        }

        code_holder.get_lines().insert(lines_iter, &line[0], &line[2]);

        break;
    }
    case 2: {
    jk_3s:
        //push reg1
        //pop reg1

        fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EBX));

        if (!IS_HAS_FULL_BITES(lines_iter->get_instruction_flags(), fuku_instruction_bad_stack_pointer)) {

            fuku_instruction line[2];

            line[0] = f_asm.push(reg1).set_custom_flags(lines_iter->get_custom_flags());
            line[1] = f_asm.pop(reg1).set_custom_flags(lines_iter->get_custom_flags());

            code_holder.get_lines().insert(lines_iter, &line[0], &line[2]);
        }
        else {
            //lea reg1, [reg1]

            code_holder.get_lines().insert(lines_iter, f_asm.lea(reg1, fuku_operand86(reg1, operand_scale::operand_scale_1, 0)).set_custom_flags(lines_iter->get_custom_flags()));
        }
        
        break;
    }

    case 3: {
        //cmp reg1, reg2

        uint32_t needed = (X86_EFLAGS_MODIFY_CF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_MODIFY_PF | X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_AF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
            fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EDI));
            fuku_reg86 reg2 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EDI));

            code_holder.get_lines().insert(lines_iter, f_asm.cmp(reg1, reg2).set_custom_flags(lines_iter->get_custom_flags()));
        }
        else {
            goto jk_2s;
        }

        break;
    }
    case 4: {
        //test reg1, reg2

        uint32_t needed = (X86_EFLAGS_MODIFY_CF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_MODIFY_PF | X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_AF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
            fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EDI));
            fuku_reg86 reg2 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EDI));

            code_holder.get_lines().insert(lines_iter, f_asm.test(reg1, reg2).set_custom_flags(lines_iter->get_custom_flags()));
        }
        else {
            goto jk_3s;
        }

        break;
    }

    }
}

void fuku_junk_3b(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator lines_iter) {

    switch (FUKU_GET_RAND(0, 3)) {
    case 0: {
        //ror reg1, 0

        fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EDI));
        code_holder.get_lines().insert(lines_iter, f_asm.ror(reg1, 0).set_custom_flags(lines_iter->get_custom_flags()));

        break;
    }
    case 1: {
        //rol reg1, 0

        fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EDI));
        code_holder.get_lines().insert(lines_iter, f_asm.rol(reg1, 0).set_custom_flags(lines_iter->get_custom_flags()));

        break;
    }
    case 2: {
        //sub reg1, 0

        uint32_t needed = (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_MODIFY_AF | X86_EFLAGS_MODIFY_PF | X86_EFLAGS_MODIFY_CF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
            code_holder.get_lines().insert(lines_iter, f_asm.sub(fuku_reg86::r_EAX, fuku_immediate86(0)).set_custom_flags(lines_iter->get_custom_flags()));
        }
        else {
            generate_junk(f_asm, code_holder, lines_iter, 2, 3);
        }
        break;
    }
    case 3: {
        //add reg1, 0

        uint32_t needed = (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_MODIFY_AF | X86_EFLAGS_MODIFY_PF | X86_EFLAGS_MODIFY_CF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
            code_holder.get_lines().insert(lines_iter, f_asm.add(fuku_reg86::r_EAX, fuku_immediate86(0)).set_custom_flags(lines_iter->get_custom_flags()));
        }
        else {
            generate_junk(f_asm, code_holder, lines_iter, 2, 3);
        }
        break;
    }
    }
}

void fuku_junk_4b(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator lines_iter) {

    switch (FUKU_GET_RAND(0, 1)) {
    case 0: {
        //not reg1
        //not reg1

        fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EBX));

        fuku_instruction line[2];

        line[0] = f_asm.not(reg1).set_custom_flags(lines_iter->get_custom_flags());
        line[1] = f_asm.not(reg1).set_custom_flags(lines_iter->get_custom_flags());


        if (reg1 == fuku_reg86::r_ESP) {
            line[1].set_instruction_flags(fuku_instruction_bad_stack_pointer);
        }

        code_holder.get_lines().insert(lines_iter, &line[0], &line[2]);

        break;
    }
    case 1: {
        //xchg reg1, reg2
        //xchg reg2, reg1

        fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_ECX, fuku_reg86::r_EBX));
        fuku_reg86 reg2 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EBX));

        fuku_instruction line[2];

        if (FUKU_GET_RAND(0, 1)) {
            line[0] = f_asm.xchg(reg1, reg2).set_custom_flags(lines_iter->get_custom_flags());
        }
        else {
            line[0] = f_asm.xchg(reg2, reg1).set_custom_flags(lines_iter->get_custom_flags());
        }

        if (FUKU_GET_RAND(0, 1)) {
            line[1] = f_asm.xchg(reg1, reg2).set_custom_flags(lines_iter->get_custom_flags());
        }
        else {
            line[1] = f_asm.xchg(reg2, reg1).set_custom_flags(lines_iter->get_custom_flags());
        }

        if (reg1 == fuku_reg86::r_ESP || reg2 == fuku_reg86::r_ESP) {
            line[1].set_instruction_flags(fuku_instruction_bad_stack_pointer);
        }

        code_holder.get_lines().insert(lines_iter, &line[0], &line[2]);

        break;
    }

    }
}

void fuku_junk_5b(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator lines_iter) {


    switch (FUKU_GET_RAND(0, 1)) {
    case 0: {
        //push reg1
        //ror reg1, rand
        //pop reg1

        uint32_t needed = (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_CF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed) &&
            !IS_HAS_FULL_BITES(lines_iter->get_instruction_flags(), fuku_instruction_bad_stack_pointer)) {

            fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EDI));
            if (reg1 == fuku_reg86::r_ESP) { reg1 = fuku_reg86::r_EAX; }

            fuku_instruction line[3];

            line[0] = f_asm.push(reg1).set_custom_flags(lines_iter->get_custom_flags());
            line[1] = f_asm.ror(reg1, FUKU_GET_RAND(1, 31)).set_custom_flags(lines_iter->get_custom_flags());
            line[2] = f_asm.pop(reg1).set_custom_flags(lines_iter->get_custom_flags());

            code_holder.get_lines().insert(lines_iter, &line[0], &line[3]);
        }
        else {
            generate_junk(f_asm, code_holder, lines_iter, 4, 5);
        }
        break;
    }
    case 1: {
        //push reg1
        //rol reg1, rand
        //pop reg1

        uint32_t needed = (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_CF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed) &&
            !IS_HAS_FULL_BITES(lines_iter->get_instruction_flags(), fuku_instruction_bad_stack_pointer)) {

            fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EDI));
            if (reg1 == fuku_reg86::r_ESP) { reg1 = fuku_reg86::r_EAX; }


            fuku_instruction line[3];

            line[0] = f_asm.push(reg1).set_custom_flags(lines_iter->get_custom_flags());
            line[1] = f_asm.rol(reg1, FUKU_GET_RAND(1, 31)).set_custom_flags(lines_iter->get_custom_flags());
            line[2] = f_asm.pop(reg1).set_custom_flags(lines_iter->get_custom_flags());

            code_holder.get_lines().insert(lines_iter, &line[0], &line[3]);
        }
        else {
            generate_junk(f_asm, code_holder, lines_iter, 4, 5);
        }
        break;
    }
    }
}

void fuku_junk_6b(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator lines_iter) {


    switch (FUKU_GET_RAND(0, 2)) {
    case 0: {
        //sub reg1(not eax), 0

        uint32_t needed = (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_MODIFY_AF | X86_EFLAGS_MODIFY_CF | X86_EFLAGS_MODIFY_PF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
            fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_ECX, fuku_reg86::r_EDI));

            code_holder.get_lines().insert(lines_iter, f_asm.sub(reg1, fuku_immediate86(0)).set_custom_flags(lines_iter->get_custom_flags()));
        }
        else {
            generate_junk(f_asm, code_holder, lines_iter, 5, 6);
        }
        break;
    }
    case 1: {
        //add reg1(not eax), 0

        uint32_t needed = (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_MODIFY_AF | X86_EFLAGS_MODIFY_CF | X86_EFLAGS_MODIFY_PF);

        if (IS_HAS_FULL_BITES(lines_iter->get_custom_flags(), needed)) {
            fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_ECX, fuku_reg86::r_EDI));

            code_holder.get_lines().insert(lines_iter, f_asm.add(reg1, fuku_immediate86(0)).set_custom_flags(lines_iter->get_custom_flags()));
        }
        else {
            generate_junk(f_asm, code_holder, lines_iter, 5, 6);
        }
        break;
    }
    case 2: {
        //jcc next_inst

        /*
        if (lines_iter != code_holder.get_lines().end()) {
            

            code_holder.get_lines().insert(lines_iter,
                f_asm.jcc((fuku_condition)FUKU_GET_RAND(0, 15), 0)
                .set_custom_flags(lines_iter->get_custom_flags()) 
            );
            
            auto jcc_iter = lines_iter; jcc_iter--;

            if (lines_iter->get_label_idx() != -1) {
                jcc_iter->set_label_idx(lines_iter->get_label_idx());
                code_holder.get_labels()[lines_iter->get_label_idx()].instruction = &(*jcc_iter);
                lines_iter->set_label_idx(-1);    
            }
            
            jcc_iter->set_rip_relocation_idx(code_holder.create_rip_relocation(2, &(*lines_iter)));
        }
        else {
            generate_junk(f_asm, code_holder, lines_iter, 5, 6);
        }*/
        generate_junk(f_asm, code_holder, lines_iter, 5, 6);
        break;
    }
    }
}


void fuku_junk_7b(fuku_asm_x86& f_asm, fuku_code_holder& code_holder, linestorage::iterator lines_iter) {

    //push reg1
    //mov reg1, randval
    //pop reg1

    if (!IS_HAS_FULL_BITES(lines_iter->get_instruction_flags(), fuku_instruction_bad_stack_pointer)) {

        fuku_reg86 reg1 = fuku_reg86(FUKU_GET_RAND(fuku_reg86::r_EAX, fuku_reg86::r_EBX));
        fuku_immediate86 imm = fuku_immediate86(FUKU_GET_RAND(0x10000000, 0xFFFFFFFF));

        fuku_instruction line[3];
        line[0] = f_asm.push(reg1).set_custom_flags(lines_iter->get_custom_flags());
        line[1] = f_asm.mov(reg1, imm).set_custom_flags(lines_iter->get_custom_flags());
        line[2] = f_asm.pop(reg1).set_custom_flags(lines_iter->get_custom_flags());

        if (FUKU_GET_RAND(0, 1)) {
            line[1].set_relocation_first_idx(code_holder.create_relocation(1, imm.get_imm(), 0));
        }

        code_holder.get_lines().insert(lines_iter, &line[0], &line[3]);
    }
    else {
        generate_junk(f_asm, code_holder, lines_iter, 6, 7);
    }

}