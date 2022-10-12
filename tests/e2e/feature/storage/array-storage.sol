// SPDX-License-Identifier: MIT
/**
This testcase targets storage vaiable in same slot

 */

// RUN: pyul %s -o %t --project-dir %S | FileCheck %s
// XFAIL: *

pragma solidity ^0.8.10;


contract ArrayTest {
    uint32 x;
    uint32 y;
    uint32[10] array;
    

    function readArray(uint256 index) external view returns (uint32){
        return array[index];
    }

    function writeArray(uint256 index, uint32 value) external {
        array[index] = value;
    }
}