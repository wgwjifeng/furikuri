#include "stdafx.h"
#include "fuku_mutation_x64.h"

#define ISNT_LAST (lines.size() > current_line_idx+1)

fuku_mutation_x64::fuku_mutation_x64(const fuku_ob_settings& settings)
: settings(settings){}

fuku_mutation_x64::~fuku_mutation_x64() {

}

void fuku_mutation_x64::obfuscate_lines(fuku_code_holder& code_holder, unsigned int recurse_idx) {

    for (linestorage::iterator lines_iter = code_holder.get_lines().begin(); lines_iter != code_holder.get_lines().end(); lines_iter++) {

        fukutation(code_holder, lines_iter);

        unsigned int recurse_idx_up = 0;
        if (recurse_idx == -1) {
            recurse_idx_up = rand() % settings.complexity + 1;
        }
        else {
            recurse_idx_up = recurse_idx - 1;
        }

        if (recurse_idx_up) {
            obfuscate_lines(code_holder, recurse_idx_up);
        }
    }
}

void fuku_mutation_x64::obfuscate(fuku_code_holder& code_holder) {
    obfuscate_lines(code_holder, -1);
}

void fuku_mutation_x64::get_junk(std::vector<uint8_t>& junk, size_t junk_size, bool unstable_stack, uint16_t allow_flags_changes) {

    /*
    size_t current_size = 0;
    linestorage lines;

    while (junk_size != current_size) {

        switch (FUKU_GET_RAND(1, min(junk_size - current_size, 7))) {
        case 1: {
            fuku_junk_1b(lines, 0, unstable_stack, allow_flags_changes); current_size += 1;
            break;
        }
        case 2: {
            fuku_junk_2b(lines, 0, unstable_stack, allow_flags_changes); current_size += 2;
            break;
        }
        case 3: {
            fuku_junk_3b(lines, 0, unstable_stack, allow_flags_changes); current_size += 3;
            break;
        }
        case 4: {
            fuku_junk_4b(lines, 0, unstable_stack, allow_flags_changes); current_size += 4;
            break;
        }
        case 5: {
            fuku_junk_5b(lines, 0, unstable_stack, allow_flags_changes); current_size += 5;
            break;
        }
        case 6: {
            fuku_junk_6b(lines, 0, unstable_stack, allow_flags_changes); current_size += 6;
            break;
        }
        case 7: {
            fuku_junk_7b(lines, 0, unstable_stack, allow_flags_changes); current_size += 7;
            break;
        }
        }
    }

    junk.resize(current_size);
    for (size_t line_idx = 0, caret_pos = 0; line_idx < lines.size(); line_idx++) {
        auto& line = lines[line_idx];
        memcpy(&junk[caret_pos], line.get_op_code(), line.get_op_length());
        caret_pos += line.get_op_length();
    }*/
}




void fuku_mutation_x64::fukutation(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    bool unstable_stack = lines_iter->get_instruction_flags() & fuku_instruction_bad_stack_pointer;

    if (FUKU_GET_CHANCE(settings.junk_chance)) {
        fuku_junk(code_holder, lines_iter);
    }

    if (FUKU_GET_CHANCE(settings.mutate_chance)) {
        switch (lines_iter->get_id()) {

        case X86_INS_PUSH: {
            fukutate_push(code_holder, lines_iter);
            break;
        }

        case X86_INS_POP: {
            fukutate_pop(code_holder, lines_iter);
            break;
        }


        case X86_INS_ADD: {
            fukutate_add(code_holder, lines_iter);
            break;
        }

        case X86_INS_SUB: {
            fukutate_sub(code_holder, lines_iter);
            break;
        }

        case X86_INS_AND: {
            fukutate_and(code_holder, lines_iter);
            break;
        }


        case X86_INS_INC: {
            fukutate_inc(code_holder, lines_iter);
            break;
        }

        case X86_INS_DEC: {
            fukutate_dec(code_holder, lines_iter);
            break;
        }

        case X86_INS_TEST: {
            fukutate_test(code_holder, lines_iter);
            break;
        }
        case X86_INS_CMP: {
            fukutate_cmp(code_holder, lines_iter);
            break;
        }

        case  X86_INS_JO: case  X86_INS_JNO:
        case  X86_INS_JB: case  X86_INS_JAE:
        case  X86_INS_JE: case  X86_INS_JNE:
        case  X86_INS_JBE:case  X86_INS_JA:
        case  X86_INS_JS: case  X86_INS_JNS:
        case  X86_INS_JP: case  X86_INS_JNP:
        case  X86_INS_JL: case  X86_INS_JGE:
        case  X86_INS_JLE:case  X86_INS_JG: {
            fukutate_jcc(code_holder, lines_iter);
            break;
        }

        case X86_INS_JMP: {
            fukutate_jmp(code_holder, lines_iter);
            break;
        }

        case X86_INS_RET: {
            fukutate_ret(code_holder, lines_iter);
            break;
        }

        }
    }

    /*
    for (auto& line : out_lines) {
        if (unstable_stack) {
            line.set_flags(line.get_flags() | fuku_instruction_bad_stack);
        }

        line.set_label_id(0);

        line.set_source_virtual_address(-1);
    }

    out_lines[0].set_source_virtual_address(lines[current_line_idx].get_source_virtual_address());
    out_lines[0].set_label_id(lines[current_line_idx].get_label_id());
    */
}



bool fuku_mutation_x64::fukutate_push(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_pop(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_add(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_sub(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_and(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_inc(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_dec(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_test(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_cmp(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_jcc(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    /*
    auto& target_line = *lines_iter;
    
    if (lines_iter++ != code_holder.get_lines().end()) {
        lines_iter--;

        const uint8_t* code = &target_line.get_op_code()[target_line.get_op_pref_size()];

        fuku_instruction l_jmp = f_asm.jmp(0);
        l_jmp.set_ip_relocation_destination(target_line.get_ip_relocation_destination());
        l_jmp.set_link_label_id(target_line.get_link_label_id());

        uint8_t cond;

        if (code[0] == 0x0F) {
            cond = code[1] & 0xF;
        }
        else {
            cond = code[0] & 0xF;
        }

        fuku_instruction l_jcc = f_asm.jcc(fuku_condition(cond ^ 1), 0).set_useless_flags(target_line.get_useless_flags());
        l_jcc.set_link_label_id(set_label(lines[current_line_idx + 1]));
        l_jcc.set_flags(l_jcc.get_flags() | fuku_instruction_full_mutated);

        out_lines.push_back(l_jcc);
        out_lines.push_back(l_jmp);
        return true;
    }
    */

    return false;
}
bool fuku_mutation_x64::fukutate_jmp(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    return false;
}
bool fuku_mutation_x64::fukutate_ret(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    /*
    auto& target_line = *lines_iter;

    if ( (target_line.get_instruction_flags() & fuku_instruction_bad_stack) == 0) {
        if (target_line.get_op_code()[target_line.get_op_pref_size()] == 0xC3) { //ret



            out_lines.push_back(f_asm.lea(fuku_reg64::r_RSP, fuku_operand64(fuku_reg64::r_RSP, 8),fuku_asm64_size::asm64_size_64));//lea rsp,[rsp + (8 + stack_offset)]
            out_lines.push_back(f_asm.jmp(fuku_operand64(r_RSP, -8)).set_instruction_flags(fuku_instruction_bad_stack));           //jmp [rsp - (8 + stack_offset)] 

            return true;

        }
        else if (target_line.get_op_code()[target_line.get_op_pref_size()] == 0xC2) { //ret 0x0000

            uint16_t ret_stack = *(uint16_t*)&target_line.get_op_code()[1];
            out_lines.push_back(f_asm.add(fuku_reg64::r_RSP, fuku_operand64(fuku_reg64::r_RSP, 8 + ret_stack), fuku_asm64_size::asm64_size_64));//lea rsp,[rsp + (8 + stack_offset)]
            out_lines.push_back(f_asm.jmp(fuku_operand64(r_RSP, -8 - ret_stack)).set_instruction_flags(fuku_instruction_bad_stack));                        //jmp [rsp - (8 + stack_offset)] 

            return true;
        }
    }
    */

    return false;
}


void fuku_mutation_x64::generate_junk(linestorage& junk,
    fuku_instruction* next_line, uint32_t max_size, size_t junk_size, bool unstable_stack, uint16_t allow_flags_changes) {

    /*
    size_t current_size = 0;

    while (junk_size != current_size) {

        switch (FUKU_GET_RAND(1, min(min(junk_size - current_size, max_size), 7))) {
        case 1: {
            fuku_junk_1b(junk, next_line, unstable_stack, allow_flags_changes); current_size += 1;
            break;
        }
        case 2: {
            fuku_junk_2b(junk, next_line, unstable_stack, allow_flags_changes); current_size += 2;
            break;
        }
        case 3: {
            fuku_junk_3b(junk, next_line, unstable_stack, allow_flags_changes); current_size += 3;
            break;
        }
        case 4: {
            fuku_junk_4b(junk, next_line, unstable_stack, allow_flags_changes); current_size += 4;
            break;
        }
        case 5: {
            fuku_junk_5b(junk, next_line, unstable_stack, allow_flags_changes); current_size += 5;
            break;
        }
        case 6: {
            fuku_junk_6b(junk, next_line, unstable_stack, allow_flags_changes); current_size += 6;
            break;
        }
        case 7: {
            fuku_junk_7b(junk, next_line, unstable_stack, allow_flags_changes); current_size += 7;
            break;
        }
        }
    }*/
}

void fuku_mutation_x64::fuku_junk(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {
    /*

    bool unstable_stack = (lines_iter->get_instruction_flags() & fuku_instruction_bad_stack);

    switch (FUKU_GET_RAND(0, 6)) {
    case 0: {
        fuku_junk_1b(code_holder, lines_iter);
        break;
    }
    case 1: {
        fuku_junk_2b(code_holder, lines_iter);
        break;
    }
    case 2: {
        fuku_junk_3b(code_holder, lines_iter);
        break;
    }
    case 3: {
        fuku_junk_4b(code_holder, lines_iter);
        break;
    }
    case 4: {
        fuku_junk_5b(code_holder, lines_iter);
        break;
    }
    case 5: {
        fuku_junk_6b(code_holder, lines_iter);
        break;
    }
    case 6: {
        fuku_junk_7b(code_holder, lines_iter);
        break;
    }
    }*/
}

void fuku_mutation_x64::fuku_junk_1b(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    code_holder.get_lines().insert(lines_iter, f_asm.nop(1));
}

void fuku_mutation_x64::fuku_junk_2b(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    switch (FUKU_GET_RAND(0, 0)) {
    case 0: {

        code_holder.get_lines().insert(lines_iter, f_asm.nop(2));
        break;
    }
    }


}
void fuku_mutation_x64::fuku_junk_3b(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    switch (FUKU_GET_RAND(0, 0)) {
    case 0: {
        code_holder.get_lines().insert(lines_iter, f_asm.nop(3));
        break;
    }
    }
}
void fuku_mutation_x64::fuku_junk_4b(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    switch (FUKU_GET_RAND(0, 0)) {
    case 0: {
        code_holder.get_lines().insert(lines_iter, f_asm.nop(4));
        break;
    }
    }
}
void fuku_mutation_x64::fuku_junk_5b(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    switch (FUKU_GET_RAND(0, 0)) {
    case 0: {
        code_holder.get_lines().insert(lines_iter, f_asm.nop(5));
        break;
    }
    }
}
void fuku_mutation_x64::fuku_junk_6b(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    switch (FUKU_GET_RAND(0, 0)) {
    case 0: {
        code_holder.get_lines().insert(lines_iter, f_asm.nop(6));
        break;
    }
    }
}
void fuku_mutation_x64::fuku_junk_7b(fuku_code_holder& code_holder, linestorage::iterator& lines_iter) {

    switch (FUKU_GET_RAND(0, 0)) {
    case 0: {
        code_holder.get_lines().insert(lines_iter, f_asm.nop(7));
        break;
    }
    }
}