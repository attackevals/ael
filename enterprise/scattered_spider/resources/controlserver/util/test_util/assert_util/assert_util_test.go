package assert_util

import (
    "testing"
)

func TestStringEquality(t *testing.T) {
    AssertEq(t, "one", "one", "Expecting string equality")
    AssertNe(t, "one", "two", "Expecting string inequality")
    AssertEq(t, "", "", "Expecting string equality")
    AssertNe(t, "", "nonempty", "Expecting string inequality")
}

func TestNumberEquality(t *testing.T) {
    AssertEq(t, 0, 0, "Expecting int equality")
    AssertEq(t, 123, 123, "Expecting int equality")
    AssertNe(t, 123, 122, "Expecting int inequality")
    AssertNe(t, 123, -123, "Expecting int inequality")
    AssertEq(t, 1.23, 1.23, "Expecting float equality")
    AssertEq(t, 1.2290, 1.229, "Expecting float equality")
    AssertNe(t, 1.23, 1.229, "Expecting float inequality")
}

func TestBoolEquality(t *testing.T) {
    AssertEq(t, true, true, "Expecting bool equality")
    AssertEq(t, false, false, "Expecting bool equality")
    AssertNe(t, false, true, "Expecting bool inequality")
}

func TestNilEquality(t *testing.T) {
    AssertEq(t, nil, nil, "Expecting nil equality")
    AssertNe(t, false, true, "Expecting strbooling inequality")
}

func TestTypeMismatch(t *testing.T) {
    AssertNe(t, "1", 1, "Expecting type inequality")
    AssertNe(t, true, "string", "Expecting type inequality")
    AssertNe(t, 0, nil, "Expecting type inequality")
    AssertNe(t, "", nil, "Expecting type inequality")
    AssertNe(t, false, nil, "Expecting type inequality")
}

func TestSliceEquality(t *testing.T) {
    // Byte slices
    AssertDeepEq(t, []byte{'\x00', '\x01', '\x02', '\x03'}, []byte{'\x00', '\x01', '\x02', '\x03'}, "Expecting byte slice equality")
    AssertDeepNe(t, []byte{'\x00', '\x01', '\x02', '\x03'}, []byte{'\x00', '\x01', '\x02'}, "Expecting byte slice inequality")
    AssertDeepNe(t, []byte{'\x00', '\x01', '\x02', '\x03'}, []byte{'\x00', '\x01', '\x02', '\x04'}, "Expecting byte slice inequality")

    // String slices
    AssertDeepEq(t, []string{"a", "b", "c"}, []string{"a", "b", "c"}, "Expecting string slice equality")
    AssertDeepNe(t, []string{"a", "b", "c"}, []string{"a", "b"}, "Expecting string slice inequality")
    AssertDeepNe(t, []string{"a", "b", "c"}, []string{"a", "b", "d"}, "Expecting string slice inequality")

    // Int slices
    AssertDeepEq(t, []int{1, 2, 3}, []int{1, 2, 3}, "Expecting int slice equality")
    AssertDeepNe(t, []int{1, 2, 3}, []int{1, 2, 3, 4}, "Expecting int slice inequality")
    AssertDeepNe(t, []int{1, 2, 3}, []int{1, 2, 4}, "Expecting int slice inequality")

    // Interface slices
    AssertDeepEq(t, []interface{}{1, "1", true, []string{"a", "b"}}, []interface{}{1, "1", true, []string{"a", "b"}}, "Expecting interface{} slice equality")
    AssertDeepNe(t, []interface{}{1, "1", true, []string{"a", "b"}}, []interface{}{2, "1", true, []string{"a", "b"}}, "Expecting interface{} slice inequality")
    AssertDeepNe(t, []interface{}{1, "1", true, []string{"a", "b"}}, []interface{}{1, "", true, []string{"a", "b"}}, "Expecting interface{} slice inequality")
    AssertDeepNe(t, []interface{}{1, "1", true, []string{"a", "b"}}, []interface{}{1, "1", false, []string{"a", "b"}}, "Expecting interface{} slice inequality")
    AssertDeepNe(t, []interface{}{1, "1", true, []string{"a", "b"}}, []interface{}{1, 1, true, []string{"a", "b"}}, "Expecting interface{} slice inequality")
    AssertDeepNe(t, []interface{}{1, "1", true, []string{"a", "b"}}, []interface{}{1, "1", true, []string{"a"}}, "Expecting interface{} slice inequality")
    AssertDeepNe(t, []interface{}{1, "1", true, []string{"a", "b"}}, []interface{}{1, "1", true, nil}, "Expecting interface{} slice inequality")

    // Empty vs non-empty slice
    AssertDeepEq(t, []byte{}, []byte{}, "Expecting slice equality")
    AssertDeepNe(t, []byte{}, []byte{'\x00'}, "Expecting slice inequality")
    AssertDeepNe(t, []byte{}, []int{}, "Expecting slice inequality")
    AssertDeepNe(t, []byte{}, nil, "Expecting slice inequality")
}

func TestMapEquality(t *testing.T) {
    // explicit value type
    AssertDeepEq(t, map[string]string{"a": "1", "b": "2", "c": "3"}, map[string]string{"b": "2", "c": "3", "a": "1"}, "")
    AssertDeepNe(t, map[string]string{"a": "1", "b": "2", "c": "3"}, map[string]string{"b": "2", "c": "3", "a": "2"}, "")
    AssertDeepNe(t, map[string]string{"a": "1", "b": "2", "c": "3"}, map[string]string{"b": "2", "c": "3", "d": "1"}, "")
    AssertDeepNe(t, map[string]string{"a": "1", "b": "2", "c": "3"}, map[string]string{"b": "2", "c": "3"}, "")
    AssertDeepNe(t, map[string]string{"a": "1", "b": "2", "c": "3"}, map[string]string{"b": "2", "c": "3", "a": "1", "d": ""}, "")
    AssertDeepNe(t, map[string]string{"a": "1", "b": "2", "c": "3"}, map[string]string{"b": "2", "c": "3", "a": "1", "d": ""}, "")
    AssertDeepNe(t, map[string]string{"a": "1", "b": "2", "c": "3"}, nil, "")

    // interface{} value
    a := map[int]interface{}{1: true, 2: 0, 3: "a", 4: []byte("test"), 5: map[string]interface{}{"1": 1, "2": "a", "3": false}}
    AssertDeepEq(t, a, map[int]interface{}{1: true, 2: 0, 3: "a", 4: []byte("test"), 5: map[string]interface{}{"1": 1, "2": "a", "3": false}}, "")
    AssertDeepNe(t, a, map[int]interface{}{-1: true, 2: 0, 3: "a", 4: []byte("test"), 5: map[string]interface{}{"1": 1, "2": "a", "3": false}}, "")
    AssertDeepNe(t, a, map[int]interface{}{1: false, 2: 0, 3: "a", 4: []byte("test"), 5: map[string]interface{}{"1": 1, "2": "a", "3": false}}, "")
    AssertDeepNe(t, a, map[int]interface{}{1: true, 2: "0", 3: "a", 4: []byte("test"), 5: map[string]interface{}{"1": 1, "2": "a", "3": false}}, "")
    AssertDeepNe(t, a, map[int]interface{}{1: true, 2: 0, 4: []byte("test"), 5: map[string]interface{}{"1": 1, "2": "a", "3": false}}, "")
    AssertDeepNe(t, a, map[int]interface{}{1: true, 2: 0, 3: "b", 4: []byte("test"), 5: map[string]interface{}{"1": 1, "2": "a", "3": false}}, "")
    AssertDeepNe(t, a, map[int]interface{}{1: true, 2: 0, 3: "a", 4: []byte("test1"), 5: map[string]interface{}{"1": 1, "2": "a", "3": false}}, "")
    AssertDeepNe(t, a, map[int]interface{}{1: true, 2: 0, 3: "a", 4: []byte("test"), 5: map[string]interface{}{"d": 1, "2": "a", "3": false}}, "")
    AssertDeepNe(t, a, map[int]interface{}{1: true, 2: 0, 3: "a", 4: []byte("test"), 5: map[string]interface{}{"1": 2, "2": "a", "3": false}}, "")
    AssertDeepNe(t, a, map[int]interface{}{1: true, 2: 0, 3: "a", 4: []byte("test"), 5: map[string]interface{}{"1": 1, "2": "a", "3": true}}, "")

    // Empty vs non-empty map
    AssertDeepEq(t, make(map[string]interface{}), make(map[string]interface{}), "Expecting map equality")
    AssertDeepNe(t, make(map[string]interface{}), map[string]interface{}{"a": "b"}, "Expecting map inequality")
    AssertDeepNe(t, make(map[string]interface{}), make(map[int]interface{}), "Expecting map inequality")
    AssertDeepNe(t, make(map[string]interface{}), nil, "Expecting map inequality")
}
