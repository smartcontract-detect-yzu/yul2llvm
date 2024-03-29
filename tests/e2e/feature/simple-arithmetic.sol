// SPDX-License-Identifier: MIT
// RUN: pyul %s -o %t --project-dir %S | FileCheck %s

pragma solidity ^0.8.10;


contract AdditionContract {
    function add(uint256 a, uint256 b) external returns (uint256) {
        return a+b;
    }
}

//CHECK: define {{.*fun_add_[0-9]+\(i256\* \%.*\)}}