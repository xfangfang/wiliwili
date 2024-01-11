//
// Created by fang on 2023/12/6.
//

#pragma once

const int PS4_MPV_SHADER_VERT_LENGTH = 561;
const int PS4_MPV_SHADER_FRAG_LENGTH = 770;

const unsigned char PS4_MPV_SHADER_VERT[] = {
    0x71, 0xbc, 0x91, 0xe8, 0x64, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x9d, 0x00, 0x00, 0x00, 0x23, 0x76, 0x65,
    0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x31, 0x30, 0x30, 0x0a, 0x61, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65,
    0x20, 0x76, 0x65, 0x63, 0x33, 0x20, 0x61, 0x50, 0x6f, 0x73, 0x3b, 0x0a, 0x61, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75,
    0x74, 0x65, 0x20, 0x76, 0x65, 0x63, 0x32, 0x20, 0x61, 0x54, 0x65, 0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 0x3b, 0x0a,
    0x76, 0x61, 0x72, 0x79, 0x69, 0x6e, 0x67, 0x20, 0x76, 0x65, 0x63, 0x32, 0x20, 0x54, 0x65, 0x78, 0x43, 0x6f, 0x6f,
    0x72, 0x64, 0x3b, 0x0a, 0x76, 0x6f, 0x69, 0x64, 0x20, 0x6d, 0x61, 0x69, 0x6e, 0x28, 0x29, 0x0a, 0x7b, 0x0a, 0x20,
    0x20, 0x20, 0x67, 0x6c, 0x5f, 0x50, 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x3d, 0x20, 0x76, 0x65, 0x63,
    0x34, 0x28, 0x61, 0x50, 0x6f, 0x73, 0x2c, 0x20, 0x31, 0x2e, 0x30, 0x29, 0x3b, 0x0a, 0x20, 0x20, 0x20, 0x54, 0x65,
    0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 0x20, 0x3d, 0x20, 0x61, 0x54, 0x65, 0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 0x3b,
    0x0a, 0x7d, 0x80, 0x01, 0x00, 0x00, 0x00, 0x04, 0xef, 0xbe, 0x9c, 0xfa, 0xdf, 0x45, 0x6e, 0x1a, 0x60, 0xba, 0x00,
    0x01, 0x00, 0x02, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x53, 0x68, 0x64, 0x72, 0x07, 0x00, 0x02, 0x00, 0x01, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x6c, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x42, 0x00,
    0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x01, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x17, 0x00, 0x02, 0x10, 0x00, 0x04, 0x03, 0x00, 0x01, 0x08, 0x02, 0x00,
    0x0f, 0x00, 0x00, 0x00, 0xff, 0x03, 0xeb, 0xbe, 0x09, 0x00, 0x00, 0x00, 0x00, 0x21, 0x80, 0xbe, 0xf2, 0x02, 0x00,
    0x7e, 0x80, 0x02, 0x02, 0x7e, 0xcf, 0x08, 0x00, 0xf8, 0x04, 0x05, 0x06, 0x00, 0x0f, 0x02, 0x00, 0xf8, 0x08, 0x09,
    0x01, 0x00, 0x00, 0x00, 0x81, 0xbf, 0x39, 0x6a, 0x06, 0x00, 0x08, 0x00, 0x00, 0x6b, 0x01, 0x03, 0x01, 0x00, 0x90,
    0x50, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x17, 0x00, 0x02, 0x10, 0x03, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0xef, 0xbe, 0x00, 0x00, 0x4f, 0x72, 0x62, 0x53, 0x68, 0x64, 0x72, 0x07, 0x45, 0x28, 0x00,
    0x00, 0x03, 0x02, 0x0c, 0x03, 0x9c, 0xfa, 0xdf, 0x45, 0x6e, 0x1a, 0x60, 0xba, 0xe9, 0x9c, 0xb6, 0xf9, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x02, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00,
    0x39, 0x00, 0x00, 0x00, 0x01, 0x24, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
    0x00, 0x01, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00, 0x03, 0x2e,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x61, 0x50, 0x6f, 0x73, 0x00,
    0x41, 0x50, 0x4f, 0x53, 0x00, 0x61, 0x54, 0x65, 0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 0x00, 0x41, 0x54, 0x45, 0x58,
    0x43, 0x4f, 0x4f, 0x52, 0x44, 0x00, 0x54, 0x65, 0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 0x00, 0x54, 0x45, 0x58, 0x43,
    0x4f, 0x4f, 0x52, 0x44, 0x00, 0x67, 0x6c, 0x5f, 0x50, 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x28, 0x6e,
    0x6f, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x29, 0x00, 0x00, 0x00};

const unsigned char PS4_MPV_SHADER_FRAG[] = {
    0x71, 0xbc, 0x91, 0xe8, 0x64, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xce, 0x00, 0x00, 0x00, 0x23, 0x76, 0x65,
    0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x31, 0x30, 0x30, 0x0a, 0x70, 0x72, 0x65, 0x63, 0x69, 0x73, 0x69, 0x6f, 0x6e,
    0x20, 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x70, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x3b, 0x0a, 0x76, 0x61, 0x72,
    0x79, 0x69, 0x6e, 0x67, 0x20, 0x76, 0x65, 0x63, 0x32, 0x20, 0x54, 0x65, 0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 0x3b,
    0x0a, 0x75, 0x6e, 0x69, 0x66, 0x6f, 0x72, 0x6d, 0x20, 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x72, 0x32, 0x44, 0x20,
    0x6f, 0x75, 0x72, 0x54, 0x65, 0x78, 0x74, 0x75, 0x72, 0x65, 0x3b, 0x0a, 0x75, 0x6e, 0x69, 0x66, 0x6f, 0x72, 0x6d,
    0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x41, 0x6c, 0x70, 0x68, 0x61, 0x3b, 0x0a, 0x76, 0x6f, 0x69, 0x64, 0x20,
    0x6d, 0x61, 0x69, 0x6e, 0x28, 0x29, 0x0a, 0x7b, 0x0a, 0x20, 0x20, 0x20, 0x67, 0x6c, 0x5f, 0x46, 0x72, 0x61, 0x67,
    0x43, 0x6f, 0x6c, 0x6f, 0x72, 0x20, 0x3d, 0x20, 0x74, 0x65, 0x78, 0x74, 0x75, 0x72, 0x65, 0x32, 0x44, 0x28, 0x6f,
    0x75, 0x72, 0x54, 0x65, 0x78, 0x74, 0x75, 0x72, 0x65, 0x2c, 0x20, 0x54, 0x65, 0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64,
    0x29, 0x3b, 0x0a, 0x20, 0x20, 0x20, 0x67, 0x6c, 0x5f, 0x46, 0x72, 0x61, 0x67, 0x43, 0x6f, 0x6c, 0x6f, 0x72, 0x2e,
    0x61, 0x20, 0x3d, 0x20, 0x41, 0x6c, 0x70, 0x68, 0x61, 0x3b, 0x0a, 0x7d, 0x0a, 0x20, 0x02, 0x00, 0x00, 0x00, 0x04,
    0xef, 0xbe, 0x7f, 0x79, 0x7f, 0x72, 0xcc, 0x71, 0x93, 0xd2, 0x01, 0x01, 0x00, 0x02, 0x04, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x68, 0x64, 0x72,
    0x07, 0x00, 0x02, 0x00, 0x02, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x50, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x81, 0x00, 0x0c, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1b, 0x01, 0x02, 0x00,
    0x00, 0x00, 0x04, 0x03, 0x01, 0x00, 0x0c, 0x00, 0x02, 0x00, 0x10, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xff, 0x03, 0xeb,
    0xbe, 0x10, 0x00, 0x00, 0x00, 0x7e, 0x04, 0x92, 0xbe, 0x7e, 0x0a, 0xfe, 0xbe, 0x10, 0x03, 0xfc, 0xbe, 0x00, 0x03,
    0x80, 0xc0, 0x00, 0x00, 0x08, 0xc8, 0x01, 0x00, 0x09, 0xc8, 0x00, 0x01, 0x0c, 0xc8, 0x01, 0x01, 0x0d, 0xc8, 0x00,
    0x07, 0x80, 0xf0, 0x02, 0x00, 0x61, 0x00, 0x7f, 0x00, 0x8c, 0xbf, 0x00, 0x01, 0x00, 0xc2, 0x7f, 0x00, 0x8c, 0xbf,
    0x00, 0x02, 0x06, 0x7e, 0x12, 0x04, 0xfe, 0xbe, 0x70, 0x0f, 0x8c, 0xbf, 0x00, 0x03, 0x00, 0x5e, 0x02, 0x07, 0x02,
    0x5e, 0x0f, 0x1c, 0x00, 0xf8, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x81, 0xbf, 0x6a, 0x08, 0x00, 0x04, 0x00, 0x51,
    0x41, 0x00, 0xcd, 0x68, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x1b, 0x01, 0x02, 0x00, 0x00, 0x00, 0x04, 0x03, 0x01,
    0x00, 0x0c, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xef, 0xbe, 0x00, 0x00,
    0x4f, 0x72, 0x62, 0x53, 0x68, 0x64, 0x72, 0x07, 0x41, 0x5c, 0x00, 0x00, 0x03, 0x04, 0x0c, 0x03, 0x7f, 0x79, 0x7f,
    0x72, 0xcc, 0x71, 0x93, 0xd2, 0x54, 0xae, 0x71, 0xe6, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x16, 0x04, 0x6a, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00,
    0x00, 0x01, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x03, 0x38,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x6f, 0x75, 0x72, 0x54, 0x65,
    0x78, 0x74, 0x75, 0x72, 0x65, 0x00, 0x5f, 0x5f, 0x47, 0x4c, 0x4f, 0x42, 0x41, 0x4c, 0x5f, 0x43, 0x42, 0x5f, 0x5f,
    0x00, 0x41, 0x6c, 0x70, 0x68, 0x61, 0x00, 0x28, 0x6e, 0x6f, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x29, 0x00, 0x54, 0x65,
    0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 0x00, 0x54, 0x45, 0x58, 0x43, 0x4f, 0x4f, 0x52, 0x44, 0x00, 0x67, 0x6c, 0x5f,
    0x46, 0x72, 0x61, 0x67, 0x43, 0x6f, 0x6c, 0x6f, 0x72, 0x00};