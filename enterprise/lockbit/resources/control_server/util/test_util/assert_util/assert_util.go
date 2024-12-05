package assert_util

import (
    "reflect"
    "testing"
)

// Asserts that primitives a and b are equal using the equality == operator.
// Terminates the calling test and prints the optional message if the two are not equal.
func AssertEq(t *testing.T, a any, b any, optionalMsg string) {
    if a != b {
        prefix := ""
        if len(optionalMsg) > 0 {
            prefix = optionalMsg + "\n"
        }
        t.Fatalf("%sExpected equality between left and right. Left: %v; Right: %v", prefix, a, b)
    }
}

// Asserts that a and b are equal using the reflect.DeepEqual() function.
// Terminates the calling test and prints the optional message if the two are not equal.
func AssertDeepEq(t *testing.T, a any, b any, optionalMsg string) {
    if !reflect.DeepEqual(a, b) {
        prefix := ""
        if len(optionalMsg) > 0 {
            prefix = optionalMsg + "\n"
        }
        t.Fatalf("%sExpected deep equality between left and right. Left: %v; Right: %v", prefix, a, b)
    }
}

// Asserts that primitives a and b are NOT equal using the equality == operator.
// Terminates the calling test and prints the optional message if the two are equal.
func AssertNe(t *testing.T, a any, b any, optionalMsg string) {
    if a == b {
        prefix := ""
        if len(optionalMsg) > 0 {
            prefix = optionalMsg + "\n"
        }
        t.Fatalf("%sExpected inequality between left and right. Left: %v; Right: %v", prefix, a, b)
    }
}

// Asserts that a and b are NOT equal NOTusing the reflect.DeepEqual() function.
// Terminates the calling test and prints the optional message if the two are equal.
func AssertDeepNe(t *testing.T, a any, b any, optionalMsg string) {
    if reflect.DeepEqual(a, b) {
        prefix := ""
        if len(optionalMsg) > 0 {
            prefix = optionalMsg + "\n"
        }
        t.Fatalf("%sExpected deep inequality between left and right. Left: %v; Right: %v", prefix, a, b)
    }
}
