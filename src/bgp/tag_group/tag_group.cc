/*
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */

#include "bgp/tag_group/tag_group.h"

#include <stdio.h>

#include <algorithm>


using std::copy;
using std::string;

TagGroup::TagGroup(as_t asn, uint32_t sgid) {
    data_[0] = 0x80;
    data_[1] = 0x04;
    put_value(&data_[2], 2, asn);
    put_value(&data_[4], 4, sgid);
}

TagGroup::TagGroup(const bytes_type &data) {
    copy(data.begin(), data.end(), data_.begin());
}

as_t TagGroup::as_number() const {
    if (data_[0] == 0x80 && data_[1] == 0x04) {
        as_t as_number = get_value(&data_[2], 2);
        return as_number;
    }
    return 0;
}

uint32_t TagGroup::tag_group_id() const {
    if (data_[0] == 0x80 && data_[1] == 0x04) {
        uint32_t num = get_value(&data_[4], 4);
        return num;
    }
    return 0;
}

bool TagGroup::IsGlobal() const {
    uint32_t sgid = tag_group_id();
    return (sgid >= kMinGlobalId && sgid <= kMaxGlobalId);
}

string TagGroup::ToString() {
    char temp[50];
    snprintf(temp, sizeof(temp), "taggroup:%u:%u",
        as_number(), tag_group_id());
    return string(temp);
}
