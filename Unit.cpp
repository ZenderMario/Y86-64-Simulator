//
// Created by ZenderMario on 2023/4/23.
//

#include "Unit.h"
#include <fstream>

namespace Unit
{


    /***********************************************************************/


    Word ANDGate::operator()(const Word &A, const Word &B) const{
        Word tmp;

        for( RangeType i = 0; i < WORDBIT; ++i)
            tmp[ i] = A[ i] & B[ i];

        return tmp;
    }

    Word ORGate::operator()(const Word &A, const Word &B) const{
        Word tmp;

        for( RangeType i = 0; i < WORDBIT; ++i)
            tmp[ i] = A[ i] | B[ i];

        return tmp;
    }

    Word NOTGate::operator()(const Word &A) const{
        Word tmp;

        for( RangeType i = 0; i < WORDBIT; ++i)
            tmp[ i] = !A[ i];

        return tmp;
    }

    Word XORGate::operator()(const Word &A, const Word &B) const{
        Word tmp;

        for( RangeType i = 0; i < WORDBIT; ++i)
            tmp[ i] = A[ i] ^ B[ i];

        return tmp;
    }
    /***********************************************************************/



    /***********************************************************************/

    AdderRes HlfAdder::operator()(bool A, bool B) const {
        AdderRes tmp;
        tmp.carry = A && B;
        tmp.sum = A^B;
        return tmp;
    }

    AdderRes FulAdder::operator()(bool A, bool B, bool C) const {
        AdderRes tmp = hlfADD( A, B);
        tmp.carry = tmp.carry || ( tmp.sum && C);
        tmp.sum = tmp.sum ^ C;
        return tmp;
    }


    Word Adder64::operator()( const Word& A, const Word& B) const{
        Word tmp;

        AdderRes res = hlfADD( A[ 63], B[ 63]);
        tmp[ 63] = res.sum;

        for( int i = 1; i < 64; i++) {
            res = fulADD( A[ 63 - i], B[ 63 - i], res.carry);
            tmp[ 63 - i] = res.sum;
        }

        return tmp;
    }

    Word Subber64::operator()(const Word &A, const Word &B) const {
        Word tmp1 = wdNOT( A);
        tmp1 = wdADD( tmp1, { 1});

        return wdADD( tmp1, B);
    }

    Word& SHL64::operator()(Word& word, RangeType bit) const{
        for( RangeType i = bit; i < WORDBIT; ++i) {
            word[ i] = word[ i - bit];
            word[ i - bit] = false;
        }

        return word;
    }

    Word& SHR64::operator()(Word& word, RangeType bit) const{
        for( RangeType i = bit; i < WORDBIT; ++i) {
            word[ i - bit] = word[ i];
            word[ i] = false;
        }

        return word;
    }

    Word& SAL64::operator()(Word& word, RangeType bit) const{
        for( RangeType i = bit; i < WORDBIT; ++i) {
            word[ i - bit] = word[ i];
            word[ i - bit] = false;
        }

        return word;
    }

    Word& SAR64::operator()(Word& word, RangeType bit) const{
        bool S = word[ 0];
        for( RangeType i = bit; i < WORDBIT; ++i) {
            word[ i - bit] = word[ i];
        }
        for( RangeType i = WORDBIT - bit; i < WORDBIT; ++i) {
            word[ i] = word[ i];
        }

        return word;
    }

    /***********************************************************************/

    Byte Locate( const Word& word, RangeType n) {
        Byte tmp;
        if( n >= 1 && n <= WORDBIT / 8) {
            for( RangeType i = 0; i < BYTEBIT; ++i)
                tmp[ i] = word[ ( n - 1) * BYTEBIT + i];
        }
        return tmp;
    }

    Byte Locate( const Address& ares, RangeType n) {
        Byte tmp;
        if( n >= 1 && n <= ADDRBIT / 8) {
            for( RangeType i = 0; i < BYTEBIT; ++i)
                tmp[ i] = ares[ ( n - 1) * BYTEBIT + i];
        }
        return tmp;
    }

    Memory::Memory() {

    }

    bool Memory::Check(Address pos, RangeType bias) {

        RangeType i = memDecoder( pos);
        if( MAXMEM - i < bias) {
            stat = MemoryStat::INVALID;
            //read invalid memory
            return false;
        }
        else
            return true;
    }



    Byte Memory::ReadB(Address pos) {
        return mem[ memDecoder( pos)];
    }
    Word Memory::ReadW( Address pos) {
        if( check)
            if(  !Check( pos, 8))
                return { 0};
        Word tmp;
        RangeType bias = memDecoder( pos);
        for( RangeType i = 0; i < WORDBIT; ++i)
            tmp[ i] = mem[ bias + i / 8][ i % 8];

        return tmp;
    }

    Byte Memory::WriteB( const Address & pos, const Byte & byte) {
        mem[ memDecoder( pos)] = byte;
        return byte;
    }
    Word Memory::WriteW( const Address & pos, const Word & word) {
        if( check)
            if(  !Check( pos, 8))
                return { 0};
        RangeType bias = memDecoder( pos);
        for( RangeType i = 0; i < WORDBIT; ++i)
            mem[ bias + i / 8][ i % 8] = word[ i];

        return word;
    }

    void Memory::Read( const Address& pos, bool * dst, Unit::RangeType size) {
        if( check)
            if(  !Check( pos, size))
                return;
        RangeType bias = memDecoder( pos);
        for( RangeType i = 0; i < size; ++i)
            for( RangeType j = 0; j < BYTEBIT; ++j)
                dst[ i * 8 + j] = mem[ bias + i][ j];

    }

    /***********************************************************************/

    Address PCIncrementer::operator()(const Address &adre, bool regiR, bool valC) {
        Word tmp = extender( adre);
        wdADD( tmp, { 1});
        wdADD( tmp, { regiR});
        Word null;
        wdADD( tmp, wdSHL( null, 3));
    }

    /***********************************************************************/

    RegidValue& Register::operator[](const RegidLabel &label) {
        RangeType r = regDecoder( label);

        if( r == 15)
            throw std::out_of_range( "Out of range!");

        return reg[ r];
    }


    Output RegisterFile::operator()(const RegidLabel &A, const RegidLabel &B, const RegidLabel &E,
                                                  const RegidLabel &M) {
        dstE = E;
        dstM = M;
        //because the other function overload needs register dstE and dstM, you should first call this to avoid
        //problems

        Word tmpA, tmpB;
        if( !regidEQU( A, NONE))
            tmpA = reg[ A];
        if( !regidEQU( B, NONE))
            tmpB = reg[ B];
        return { tmpA, tmpB};
    }

    void RegisterFile::operator()(const Word &valE, const Word &valM, bool cnd) {


        if( cnd && !regidEQU( dstE, NONE))
            reg[ dstE] = valE;
        if( cnd && !regidEQU( dstM, NONE))
            reg[ dstM] = valM;
    }

    /***********************************************************************/

    /***********************************************************************/

    Word ALU::operator()(const Word &A, const Word &B, const Bit< 4>& ifunc) {

        RangeType codeR = aluDecoder( ifunc);

        Word ret;

        switch( codeR) {
            case 0:
                ret = wdADD( A, B);break;
            case 1:
                ret = wdSUB( A, B);break;
            case 2:
                ret = wdAND( A, B);break;
            case 3:
                ret = wdXOR( A, B);break;
            default:
                throw std::runtime_error( "Invalid Instruction!");
        }
        return ret;
    }

}