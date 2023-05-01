//
// Created by ZenderMario on 2023/4/22.
//

#include "SEQProcessor.h"
#include <iostream>
#include <map>
#include "Reader.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>

namespace SEQ {
    /***********************************************************************/

    void SeqProcessor::Print() {

        Unit::Incrementer<4> codeIncre;
        Unit::EQUGate<4> cmp;

        Unit::RegidLabel r = {0, 0, 0, 0};
        Unit::RegidLabel end = { 1, 1, 1, 1};
        for (; !cmp(r, end); r = codeIncre(r)) {
            std::cout << "Register " << icodeDecoder(r) << " :";
            reg[r].Print();
        }

        std::cout << "PC :";
        PC.Print();

        std::cout << "Code :";
        IR.icode.Print();
        std::cout << "Func :";
        IR.ifunc.Print();

        std::cout << std::boolalpha;
        std::cout << "Need Rigister: " << ctrlUnit.needRegis << '\n';
        std::cout << "Need Constants: " << ctrlUnit.needCons << '\n';
        std::cout << "Instruction Memory Error: " << ctrlUnit.imemErr << '\n';
        std::cout << "Data Memory Error: " << ctrlUnit.dmemErr << '\n';
        std::cout << "Instruction Valid: " << ctrlUnit.instrInvalid << '\n';
        std::cout << "Condition Code: " << ctrlUnit.cnd << '\n';

        std::cout << std::noboolalpha;
    }



    void SeqProcessor::Fetch( const Bit< 10 * BYTEBIT>& ins, Word& valP, RegidLabel& A,
                              RegidLabel& B, Word& s) {
        IR.icode.Set( ins.loca());
        IR.ifunc.Set( ins.loca() + 4);

        RangeType codeR = 0;
        codeR = icodeDecoder( IR.icode);

        A.Set( ins.loca() + 8);
        B.Set( ins.loca() + 12);

        switch( codeR)
        {
            case 0:
                Stat = Unit::consStat[ 1];
                break;
                //HLT
            case 1:
                Stat = Unit::consStat[ 4];
                break;
                //NOP
            case 2:
                ctrlUnit.needCons  = false;
                ctrlUnit.needRegis = true;
                break;
                //cmove
            case 3:
                //irmov
            case 4:
                //rmmov
            case 5:
                s.Set( ins.loca() + 16);
                ctrlUnit.needCons  = true;
                ctrlUnit.needRegis = true;break;
                //mrmov
            case 6:
                ctrlUnit.needCons  = false;
                ctrlUnit.needRegis = true;break;
                //OP
            case 7:
                //jxx
            case 8:
                s.Set( ins.loca() + 8);
                ctrlUnit.needCons  = true;
                ctrlUnit.needRegis = false;break;
                //call
            case 9:
                ctrlUnit.needCons  = false;
                ctrlUnit.needRegis = false;break;
                //ret
            case 10:
                ctrlUnit.needCons  = false;
                ctrlUnit.needRegis = true;break;
                //push
            case 11:
                ctrlUnit.needCons  = false;
                ctrlUnit.needRegis = true;break;
                //pop
            case 12:
                ctrlUnit.needRegis = true;
                ctrlUnit.needCons  = false;
        }
        valP = { false};
        valP = Unit::wdADD( { ctrlUnit.needRegis}, { 1});
        valP = Unit::wdADD( valP, { ctrlUnit.needCons, 0, 0, 0});
        valP = Unit::wdADD( valP, Unit::ExtendTo64U< Unit::ADDRBIT>()( PC));
    }

    void SeqProcessor::Decode( const RegidLabel& A, const RegidLabel& B) {
        RangeType codeR = 0;
        codeR = icodeDecoder( IR.icode);

        RegidLabel srcA, srcB;
        RegidLabel dstE, dstM;

        if( codeR == 2 || codeR == 4 || codeR == 6 || codeR == 10 || codeR == 12)
            srcA = A;
        else if( codeR == 9 || codeR == 11)
            srcA = RRSP; //%rsp
        else
            srcA = NONE; //no register


        if( codeR == 6 || codeR == 4 || codeR == 5)
            srcB = B;
        else if( codeR == 9)
            srcB = RRSP;
        else
            srcB = NONE;


        if( codeR == 2 || codeR == 3 || codeR == 6)
            dstE = B;
        else if( codeR == 10 || codeR == 11 || codeR == 8 || codeR == 9)
            dstE = RRSP;
        else
            dstE = NONE;
        //though cmov may influence the choice of the dstE
        //we adopt the strategy that leave this to ACCESS

        if( codeR == 11)
            dstM = A;
        else
            dstM = NONE;


        outputReg = regFile( srcA, srcB, dstE, dstM);
    }

    Word SeqProcessor::Execute( const Word& cons) {
        Word aluA, aluB;

        RangeType codeR = icodeDecoder( IR.icode);

        const Word wdNeg8 = {
                1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
                1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
                1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
                1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 0, 0, 0
        };

        if( codeR == 0 || codeR == 1)
            return {0};

        if( codeR == 2 || codeR == 6 || codeR == 12)
            aluA = outputReg.A;
        else if( codeR == 3 || codeR == 4 || codeR == 5)
            aluA = cons;
        else if( codeR == 8 || codeR == 10)
            aluA = wdNeg8;
        else if( codeR == 9 || codeR == 11)
            aluA = { 1, 0, 0, 0, 0};

        if( codeR == 2 || codeR == 3 || codeR == 12)
            aluB = { 0};
        else
            aluB = outputReg.B;

        Word tmp;
        if( codeR == 6)
            tmp = aluUnit( aluA, aluB, IR.ifunc);
        else
            tmp = aluUnit( aluA, aluB, { 0, 0, 0, 0});
            //other instruction always adopt add

        if( codeR == 6) {
            condCode.Reset();
            RangeType funcR = icodeDecoder( IR.ifunc);

            if( tmp[ 0])
                condCode.SetSF();
            if( Unit::wdEQU( tmp, { 0}))
                condCode.SetZF();

            switch( funcR + aluA[ 0] * 2 + aluB[ 0] * 4 + tmp[ 0] * 8) {
                case 6:
                case 8:
                case 11:
                case 13:
                    condCode.SetOF();
                default:
                    break;
            }

            /*
             * old version of decision
            if( funcR == 0 && ( !aluA[ 0] && !aluB[0] && tmp[ 0]))
                condCode.SetOF(); //overflow
            else if( funcR == 1 && ( aluA[ 0] && !aluB[0] && tmp[ 0]))
                condCode.SetOF(); //overflow
            else if( funcR == 0 && ( aluA[ 0] && aluB[0] && !tmp[ 0]))
                condCode.SetOF(); //underflow
            else if( funcR == 1 && ( !aluA[ 0] && aluB[0] && tmp[ 0]))
                condCode.SetOF(); //underflow
            */
        }

        if( codeR == 2 || codeR == 7) {
            bool trans = true;


            switch( icodeDecoder( IR.ifunc)) {
                case 0:
                    break;
                case 1:
                    if ( condCode.L())
                        trans = false;
                    break;
                case 2:
                    if (condCode.LE())
                        trans = false;
                    break;
                case 3:
                    if (condCode.E())
                        trans = false;
                    break;
                case 4:
                    if (condCode.NE())
                        trans = false;
                    break;
                case 5:
                    if (condCode.GE())
                        trans = false;
                    break;
                case 6:
                    if (condCode.G())
                        trans = false;
                    break;
                default:
                    ctrlUnit.instrInvalid = true;
                    break;
                }
            ctrlUnit.cnd = trans;
        }


        return tmp;

    }

    void SeqProcessor::Access( const Word& valE, const Word& valA, const Word& valP, Word& valM) {
        Address addr;
        Word data;

        RangeType codeR = icodeDecoder( IR.icode);
        bool memRead = false;
        bool memWrite = false;


        if( codeR == 4 || codeR == 8 || codeR == 10)
            memWrite = true;

        if( codeR == 5 || codeR == 9 || codeR == 11)
            memRead = true;

        if( codeR == 4 || codeR == 10 || codeR == 8 || codeR == 5)
            addr.Set( valE.loca() + 48);
        else if( codeR == 11 || codeR == 9)
            addr.Set( valA.loca() + 48);

        if( codeR == 4 || codeR == 10)
            data = valA;
        else if( codeR == 8)
            data = valP;
        //choose the data to write later

        if( memRead){
            valM = mem.ReadW( addr);
            if( mem.Bad())
                ctrlUnit.dmemErr = true;
        }

        if(memWrite) {
            mem.WriteW(addr, data);
            if( mem.Bad())
                ctrlUnit.dmemErr = true;
        }

        Stat[ 2] = ctrlUnit.imemErr | ctrlUnit.dmemErr;
        Stat[ 3] = ctrlUnit.instrInvalid;
        Stat[ 1] = Stat[ 2] | Stat[ 3];
    }

    void SeqProcessor::Write( const Word& valE, const Word& valM) {
        RangeType codeR = icodeDecoder( IR.icode);
        bool cnd = true;
        if( codeR == 2 || codeR == 7)
            cnd = ctrlUnit.cnd;
        if( codeR != 12)
            regFile( valE, valM, cnd);
        else
            valE.Print();


    }

    void SeqProcessor::Update( const Word& valC, const Word& valM, const Word& valP) {
        switch( icodeDecoder( IR.icode)) {
            case 7:
                if( ctrlUnit.cnd)
                    PC.Set( valC.loca() + 48);
                else
                    PC.Set( valP.loca() + 48);
                break;
            case 8:
                PC.Set( valC.loca() + 48);
                break;
                //call
            case 9:
                PC.Set( valC.loca() + 48);
                break;
                //ret
            default:
                PC.Set( valP.loca() + 48);;
        }
    }

    void SeqProcessor::Run() {
        //the processes a SEQ needs are divided to six parts
        //fetch - get instruction from memory PC indicates and split it to several parts
        //decode - get value from registers
        //execute - do operations in ALU and set CC, prepare for next step
        //access memory - read/ write from/ to memory( data memory, is in same part as instruction, but with different purpose
        //write back - write value calculated to registers
        //update - update PC, the address depends on decisions made previously

        using Unit::RegidLabel;
        using Unit::Word;
        using Unit::Code;
        using instrL = Unit::Bit< 10 * Unit::BYTEBIT>;

        using Unit::wdEQU;
        bool quitFlag = false;
        RegidLabel regA, regB;
        Word cons;
        Word valE;
        Word valM;
        Word valP;
        //those value are prepared to serve for splitting and other steps

        Unit::EQUGate< 10 * Unit::BYTEBIT> insEQU;

        Translates();

        std::cout << "PC :" << std::endl;

        while( !quitFlag) {
            instrL ins;

            mem.Read( PC, ins.loca(), 10);
            //read instructions, some needs 10 bytes to code

            PC.Print();

            if( insEQU( ins, { 0})) {
                quitFlag = true;
            }

            Fetch( ins, valP, regA, regB, cons);
            //align

            Decode( regA, regB);
            valE = Execute( cons);
            Access( valE, outputReg.A, valP, valM);
            Write( valE, valM);

            Update( cons, valM, valP);
        }

        if( isPrint)
            Print();

    }

    Word SeqProcessor::ConvertW( const std::string& s) {
        Word tmp;
        char * str = new char[ s.size() + 1];
        strncpy( str, s.c_str(), s.size() + 1);
        uint64_t conV = strtoull( str, nullptr, 0);
        delete[] str;
        return Unit::wdCoder( conV);
    }

    Address SeqProcessor::ConvertA( const std::string& s) {
        Address tmp;
        char * str = new char[ s.size() + 1];
        strncpy( str, s.c_str(), s.size() + 1);
        uint64_t conV = strtoul( str, nullptr, 0);
        delete[] str;
        return Unit::Coder< 16>()( conV);
    }

    std::string SeqProcessor::Element( RangeType countL, const std::string &str, const std::string flag1, uint8_t bias1, const std::string flag2,
                                      uint8_t bias2, bool rFind1, bool rFind2) {
        RangeType pos1, pos2;
        if( flag1 == "")
            pos1 = 0;
        else if( rFind1)
            pos1 = str.find_last_of( flag1);
        else
            pos1 = str.find_first_of( flag1);

        if( flag2 == "")
            pos2 = str.size();
        else if( rFind2)
            pos2 = str.find_last_of( flag2);
        else
            pos2 = str.find_first_of( flag2);

        if( pos1 == -1 || pos2 == -1)
            ErrorMessage( "Error in Element Spilt", countL);

        pos1 += bias1;
        pos2 += bias2;

        if( pos1 < 0 || pos1 > str.size() - 1 || pos2 < 0 || pos2 > str.size())
            ErrorMessage( "Error in Instruction Detect", countL);
        std::string tmp = std::string( str.c_str() + pos1, str.c_str() + pos2);
        tmp.erase( std::remove( tmp.begin(), tmp.end(), ' '), tmp.end());
        return tmp;
    }

    void SeqProcessor::Translates() {
        using Unit::CODE;
        using Unit::FUNC;

        Unit::Code none = { 0, 0, 0, 0};

        std::map< std::string, Unit::Address> labelMap;
        std::map< std::string, Unit::RegidLabel> regidMap = {
                { "rax", { 0, 0, 0, 0}},
                { "rbx", { 0, 0, 0, 1}},
                { "rcx", { 0, 0, 1, 0}},
                { "rdx", { 0, 0, 1, 1}},
                { "rsp", { 0, 1, 0, 0}},
                { "rbp", { 0, 1, 0, 1}},
                { "rsi", { 0, 1, 1, 0}},
                { "rdi", { 0, 1, 1, 1}},
                { "r8", { 1, 0, 0, 0}},
                { "r9", { 1, 0, 0, 1}},
                { "r10", { 1, 0, 1, 0}},
                { "r11", { 1, 0, 1, 1}},
                { "r12", { 1, 1, 0, 0}},
                { "r13", { 1, 1, 0, 1}},
                { "r14", { 1, 1, 1, 0}}
        };
        std::map< std::string, Unit::Byte> instrMap  = {
                { "hlt", CODE[ 0] + none},
                { "nop", CODE[ 1] + none},
                { "rrmov", CODE[ 2] + FUNC[ 2][ 0]},
                { "cmovl", CODE[ 2] + FUNC[ 2][ 1]},
                { "cmovle", CODE[ 2] + FUNC[ 2][ 2]},
                { "cmove", CODE[ 2] + FUNC[ 2][ 3]},
                { "cmovne", CODE[ 2] + FUNC[ 2][ 4]},
                { "cmovge", CODE[ 2] + FUNC[ 2][ 5]},
                { "cmovg", CODE[ 2] + FUNC[ 2][ 6]},
                { "irmov", CODE[ 3] + none},
                { "rmmov", CODE[ 4] + none},
                { "mrmov", CODE[ 5] + none},
                { "add", CODE[ 6] + FUNC[ 0][ 0]},
                { "sub", CODE[ 6] + FUNC[ 0][ 1]},
                { "and", CODE[ 6] + FUNC[ 0][ 2]},
                { "xor", CODE[ 6] + FUNC[ 0][ 3]},
                { "jmp", CODE[ 7] + FUNC[ 1][ 0]},
                { "jl", CODE[ 7] + FUNC[ 1][ 1]},
                { "jle", CODE[ 7] + FUNC[ 1][ 2]},
                { "je", CODE[ 7] + FUNC[ 1][ 3]},
                { "jne", CODE[ 7] + FUNC[ 1][ 4]},
                { "jge", CODE[ 7] + FUNC[ 1][ 5]},
                { "jg", CODE[ 7] + FUNC[ 1][ 6]},
                { "call", CODE[ 8] + none},
                { "ret", CODE[ 9] + none},
                { "push", CODE[ 10] + none},
                { "pop", CODE[ 11] + none},
                { "PRINT", CODE[ 12] + none}
        };

        auto FINDI = [&instrMap, this]( const std::string& ins, RangeType countL)
        {
            if( instrMap.find( ins) == instrMap.end())
                ErrorMessage( "Invalid Instruction", countL);
            else
                return instrMap[ ins];
        };
        //used to find specific instruction code

        auto FINDR = [&regidMap, this]( const std::string& ins, RangeType countL)
        {
            if( regidMap.find( ins) == regidMap.end())
                ErrorMessage( "Invalid Register", countL);
            else
                return regidMap[ ins];
        };

        auto FINDL = [&labelMap, this]( const std::string& ins, RangeType countL)
        {
            if( labelMap.find( ins) == labelMap.end())
                ErrorMessage( "Invalid Label", countL);
            else
                return labelMap[ ins];
        };

        std::ifstream is;
        std::string name;
        std::cout << "Please make sure there is file in your source file folder" << std::endl;
        //std::getline( std::cin, name);

        Reader::FileReader file( std::string( "Code.txt"));

        Address adre;
        Incrementer< ADDRBIT> adreIncre;

        RangeType pos1, pos2;//to split the string
        std::string ele1, ele2;
        std::string ins;

        file.Read( ins);
        ele1 = Element( 0, ins, "=", 1, "", 0);
        reg[ RRSP] = ConvertW( ele1);
        //set stack size

        isPrint = false;
        //print registers and instructions in Y86
        file.Read( ins);
        ele1 = Element( 1, ins, "=", 1, "", 0);
        if( ele1 == "ON")
            isPrint = true;
        else if( ele1 != "OFF")
            throw std::runtime_error( "Setting Error in PRINT");

        std::map< std::string, Word>::iterator iter;

        file.Read( ins);
        RangeType countL = 2;

        while( ins != "END") {
            ++countL;

            if( ins == "") {
                file.Read( ins);
                continue;
            }

            if( ins[ 0] == '#') {
                //function definition
                ele1 = Element( countL, ins, "#", 1, "=", 0);
                ele2 = Element( countL, ins, "=", 1, "", 0);

                //(#FUNC=0x1acd)
                //ele1 = FUNC -- label name
                //ele2 = 0x1acd -- label address

                adre = ConvertA(ele2);
                labelMap.insert( { ele1, adre});
            }
            else if( ins[ 0] == '.') {

                ele1 = Element( countL, ins, ".", 1, ":", 0);
                //.L1  -- an address flag, used by jmp
                //ele1 = L1 -- label name
                labelMap.insert({ele1, adre});
            }else {

                if( ins == "hlt" || ins == "nop" || ins == "ret") {
                    adre = adreIncre( adre);
                    file.Read(ins);

                    continue;
                }

                ele1 = Element( countL, ins, "", 0, "%$.", 0);
                Code tmp;
                tmp.Set( FINDI( ele1, countL).loca());
                RangeType incre = 0;
                switch( icodeDecoder( tmp)) {
                    case 2:
                        incre = 2;break;
                    case 3:
                        incre = 10;break;
                    case 4:
                        incre = 10;break;
                    case 5:
                        incre = 10;break;
                    case 6:
                        incre = 2;break;
                    case 7:
                        incre = 9;break;
                    case 12:
                        incre = 2;break;
                }
                adre = adreIncre( adre, incre);
            }

            file.Read( ins);
        }

        auto p = labelMap.find( "MAIN");
        if( p == labelMap.end() || Decoder< ADDRBIT>()( p->second) != 0)
            throw std::runtime_error( "Invalid Program Entrance!");

        file.ReWind();
        file.Read( ins);
        adre = Address();

        Word conV;
        Unit::ExtendTo64U< ADDRBIT> extender;

        file.Read( ins);
        file.Read( ins);
        file.Read( ins);
        countL = 2;

        while( ins != "END") {
            ++countL;
            if( ins == "" || ins[ 0] == '.') {
                file.Read( ins);


                continue;
            }
            else if( ins[ 0] == '#') {
                ele1 = Element( countL, ins, "#", 1, "=", 0);
                adre = FINDL( ele1, countL);

                file.Read( ins);
                continue;
            }
            else if( ins == "hlt" || ins == "nop" || ins == "ret") {
                mem.WriteB( adre, FINDI( ins, countL));
                adre = adreIncre( adre);
                file.Read( ins);

                continue;
            }
            else{
                ele1 = Element( countL, ins, "", 0, "%$.", 0);
                mem.WriteB( adre, FINDI( ele1, countL));
                adre = adreIncre( adre);

                Code icode;
                icode.Set( FINDI( ele1, countL).loca());
                RangeType codeR = icodeDecoder( icode);

                if(  codeR == 7 || codeR == 8) {
                    ele1 = Element( countL, ins, ".", 1, "", 0);
                    mem.WriteW( adre, extender( FINDL( ele1, countL)));
                    //jmp .L1
                    adre = adreIncre( adre, 8);


                }else if( codeR == 10 || codeR == 11 || codeR == 12) {
                    ele1 = Element( countL, ins, "%", 1, "", 0);
                    mem.WriteB( adre, FINDR( ele1, countL) + NONE);
                    adre = adreIncre( adre);
                }
                else {
                    ele1 = Element( countL, ins, "$%", 1, ",", 0);
                    ele2 = Element( countL, ins, "$%", 1, "", 0, true);

                    switch( codeR) {
                        case 2:
                            mem.WriteB(adre, FINDR(ele1, countL) + FINDR(ele2, countL));
                            adre = adreIncre(adre);
                            //rrmov %rsp, %rdx
                            break;
                        case 3:
                            mem.WriteB(adre, NONE + FINDR(ele2, countL));
                            adre = adreIncre(adre);
                            mem.WriteW(adre, ConvertW(ele1));
                            adre = adreIncre(adre, 8);
                            //irmov $64, %rdx
                            break;
                        case 4:
                            ele2 = Element( countL, ins, "%", 1, ")", 0); //rb
                            mem.WriteB(adre, FINDR(ele1, countL) + FINDR(ele2, countL));
                            adre = adreIncre(adre);

                            ele1 = Element( countL, ins, "$", 1, "(", 0); //cons
                            mem.WriteW(adre, ConvertW(ele1));
                            adre = adreIncre(adre, 8);
                            //mrmov D(%rbp), %rdx
                            break;
                        case 5:
                            ele1 = Element( countL, ins, "%", 1, ")", 0); //ra
                            mem.WriteB(adre, FINDR(ele2, countL) + FINDR(ele1, countL));
                            adre = adreIncre(adre);

                            ele1 = Element( countL, ins, "$", 1, "(", 0);
                            mem.WriteW(adre, ConvertW(ele1));
                            adre = adreIncre(adre, 8);
                            //mrmov D(%rbp), %rdx
                            break;
                        case 6:
                            mem.WriteB(adre, FINDR( ele1, countL) + FINDR( ele2, countL));
                            adre = adreIncre(adre);
                            break;
                        }
                    }



            }

            file.Read( ins);
        }

    }
    //read label and address


    void SeqProcessor::ErrorMessage(const std::string &str, RangeType line) {
        std::string tmp;
        if( line == 0)
            throw std::runtime_error( str + "  Line:" + tmp);

        while( line > 0) {
            tmp += (line % 10 + '0');
            line /= 10;
        }

        std::string t( tmp.rbegin(), tmp.rend());
        throw std::runtime_error( str + "  Line:" + tmp);
    }

    /***********************************************************************/

}