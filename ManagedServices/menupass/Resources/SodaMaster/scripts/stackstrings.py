# ---------------------------------------------------------------------------
# stackstrings.py  - Used to create stack strings for SodaMaster.

# Copyright 2023 MITRE Engenuity. Approved for public release. Document number CT0005.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: stackstrings.py --function_name [function name] --variable_name [var] --string [string to stackstring]

# Revision History:

# --------------------------------------------------------------------------- 
import random, string, argparse

# arguments
parser = argparse.ArgumentParser()
parser.add_argument("--function_name", help="name of the utility function in c code", default="RetCharVal", required=False)
parser.add_argument("--variable_name", help="name of the char array variable that will host the string", default="TestString", required=False)
parser.add_argument("--string", help="unobuscated string", default="http://google.com", required=True)
args = parser.parse_args()

def split_char_value(character):
    char_num=ord(character)
    value_1 = random.randrange(char_num)
    value_2 = char_num - value_1
    return value_1, value_2


function_name = args.function_name
variable_name = args.variable_name
original_string = args.string
str_len = len(original_string)
modified_string = ""

stack_string = ""
stack_string += 'char {0}[] = "[PLACE_STRING]";\n'.format(variable_name)

count = 0
for c in original_string:
    if count == 0:
        prestring = "*{0} = ".format(variable_name)
    else:
        prestring = "*({0} + {1}) = ".format(variable_name, count)
    rand_int = random.randrange(100)
    if rand_int <= 65:
        # use the function to stack
        values = split_char_value(c)

        stack_string += "{0}{1}({2},{3});\n".format(prestring, function_name, values[0] + 5, values[1])
        modified_string += random.choice(string.ascii_letters)
    elif 85 > rand_int > 65:
        # use original value but still stack it
        stack_string += '{0}{1};\n'.format(prestring, ord(c))
        modified_string += random.choice(string.ascii_letters)
    else:
        # use the unmodified value in the string
        modified_string += c

    count += 1

stack_string = stack_string.replace("[PLACE_STRING]", modified_string)
print(stack_string)