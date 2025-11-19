#!/usr/bin/env python3
"""
C Function Splitter

This script splits a C source file into multiple files, with each function
in its own separate file. It preserves includes, global variables, and
other declarations in each output file.

Usage: python c_function_splitter.py <input_file.c> [output_directory]
"""

import re
import os
import sys
from pathlib import Path
from typing import List, Tuple, Dict


class CFunctionSplitter:
    def __init__(self):
        # Regex patterns for C parsing
        self.function_pattern = re.compile(
            r'^((?:static\s+)?[a-zA-Z_][a-zA-Z0-9_\s\*]*)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*(?:__[a-zA-Z_]+\s*\([^)]*\))?\s*\{',
            re.MULTILINE
        )
        self.include_pattern = re.compile(r'^\s*#include\s+[<"][^>"]+[>"]', re.MULTILINE)
        self.define_pattern = re.compile(r'^\s*#define\s+.*$', re.MULTILINE)
        self.typedef_pattern = re.compile(r'^\s*typedef\s+.*?;', re.MULTILINE | re.DOTALL)
        self.global_var_pattern = re.compile(
            r'^\s*(?:static\s+)?(?:volatile\s+)?(?:const\s+)?[a-zA-Z_][a-zA-Z0-9_\s\*]*\s+[a-zA-Z_][a-zA-Z0-9_]*(?:\[[^\]]*\])?\s*(?:=\s*[^;]+)?;',
            re.MULTILINE
        )

    def find_matching_brace(self, content: str, start_pos: int) -> int:
        """Find the matching closing brace for an opening brace at start_pos"""
        brace_count = 0
        i = start_pos

        while i < len(content):
            if content[i] == '{':
                brace_count += 1
            elif content[i] == '}':
                brace_count -= 1
                if brace_count == 0:
                    return i
            elif content[i] == '"':
                # Skip string literals
                i += 1
                while i < len(content) and content[i] != '"':
                    if content[i] == '\\':
                        i += 1  # Skip escaped character
                    i += 1
            elif content[i] == "'":
                # Skip character literals
                i += 1
                while i < len(content) and content[i] != "'":
                    if content[i] == '\\':
                        i += 1  # Skip escaped character
                    i += 1
            elif content[i:i+2] == '//':
                # Skip single-line comments
                while i < len(content) and content[i] != '\n':
                    i += 1
                continue
            elif content[i:i+2] == '/*':
                # Skip multi-line comments
                i += 2
                while i < len(content) - 1 and content[i:i+2] != '*/':
                    i += 1
                i += 1  # Skip the '*' in '*/'
            i += 1

        return -1  # No matching brace found

    def extract_functions_preserve_static(self, content: str) -> List[Tuple[str, str, int, int]]:
        """Extract all functions from the C content, preserving static keywords"""
        functions = []

        for match in self.function_pattern.finditer(content):
            return_type = match.group(1).strip()
            func_name = match.group(2).strip()
            start_pos = match.start()

            # Find the opening brace
            brace_pos = content.find('{', match.end() - 1)
            if brace_pos == -1:
                continue

            # Find the matching closing brace
            end_brace = self.find_matching_brace(content, brace_pos)
            if end_brace == -1:
                continue

            # Extract the complete function WITHOUT removing static
            func_content = content[start_pos:end_brace + 1]
            functions.append((func_name, func_content, start_pos, end_brace + 1))

        return functions

    def extract_functions(self, content: str) -> List[Tuple[str, str, int, int]]:
        """Extract all functions from the C content"""
        functions = []

        for match in self.function_pattern.finditer(content):
            return_type = match.group(1).strip()
            func_name = match.group(2).strip()
            start_pos = match.start()

            # Find the opening brace
            brace_pos = content.find('{', match.end() - 1)
            if brace_pos == -1:
                continue

            # Find the matching closing brace
            end_brace = self.find_matching_brace(content, brace_pos)
            if end_brace == -1:
                continue

            # Extract the complete function
            func_content = content[start_pos:end_brace + 1]

            # Remove 'static' keyword from function definition for split files
            func_content = self.remove_static_keyword(func_content)

            functions.append((func_name, func_content, start_pos, end_brace + 1))

        return functions

    def remove_static_keyword(self, func_content: str) -> str:
        """Remove 'static' keyword from function definition"""
        lines = func_content.split('\n')
        for i, line in enumerate(lines):
            # Look for the function definition line (contains function name and parameters)
            if ('(' in line and ')' in line) or ('{' in line):
                # Remove 'static' keyword but preserve other keywords
                lines[i] = re.sub(r'\bstatic\s+', '', line)
                if '{' in line:  # If opening brace is on same line, we're done
                    break
        return '\n'.join(lines)

    def extract_header_content(self, content: str, exclude_global_vars: bool = False) -> str:
        """Extract includes, defines, typedefs, and optionally global variables"""
        header_parts = []

        # Extract includes
        includes = self.include_pattern.findall(content)
        if includes:
            header_parts.extend(includes)
            header_parts.append("")  # Empty line after includes

        # Extract defines
        defines = self.define_pattern.findall(content)
        if defines:
            header_parts.extend(defines)
            header_parts.append("")  # Empty line after defines

        # Extract typedefs
        typedefs = self.typedef_pattern.findall(content)
        if typedefs:
            header_parts.extend(typedefs)
            header_parts.append("")  # Empty line after typedefs

        # Extract global variables (before any function) only if not excluded
        if not exclude_global_vars:
            functions = self.extract_functions(content)
            if functions:
                first_func_start = min(func[2] for func in functions)
                pre_function_content = content[:first_func_start]
            else:
                pre_function_content = content

            # Find global variables in pre-function content
            lines = pre_function_content.split('\n')
            global_vars = []

            for line in lines:
                line = line.strip()
                if (line and
                    not line.startswith('#') and
                    not line.startswith('//') and
                    not line.startswith('/*') and
                    ';' in line and
                    not 'typedef' in line and
                    not 'extern' in line):

                    # More specific pattern for global variables
                    if re.match(r'^\s*(?:static\s+)?(?:volatile\s+)?(?:const\s+)?(?:uint8_t|uint16_t|uint32_t|int8_t|int16_t|int32_t|bool|char|int|float|double)\s+(?:__xdata\s+)?[a-zA-Z_][a-zA-Z0-9_]*(?:\[[^\]]*\])?\s*(?:=.*)?;', line):
                        global_vars.append(line)

            if global_vars:
                header_parts.extend(global_vars)
                header_parts.append("")  # Empty line after global vars

        return '\n'.join(header_parts)

    def extract_global_variables(self, content: str) -> List[str]:
        """Extract global variable definitions from content (only from global scope)"""
        global_vars = []

        # Find the first function to determine where global scope ends
        functions = self.extract_functions(content)
        if functions:
            first_func_start = min(func[2] for func in functions)
            global_scope_content = content[:first_func_start]
        else:
            global_scope_content = content

        # Track brace depth to ensure we're only looking at global scope
        lines = global_scope_content.split('\n')
        brace_depth = 0

        for line in lines:
            line_stripped = line.strip()

            # Track brace depth
            brace_depth += line.count('{') - line.count('}')

            # Only process lines at global scope (brace_depth == 0)
            if (brace_depth == 0 and
                line_stripped and
                not line_stripped.startswith('#') and
                not line_stripped.startswith('//') and
                not line_stripped.startswith('/*') and
                ';' in line_stripped and
                not 'typedef' in line_stripped and
                not 'extern' in line_stripped and
                not '(' in line_stripped and  # Exclude function declarations
                not 'return' in line_stripped):  # Exclude return statements

                # Check if it looks like a global variable declaration
                if re.match(r'^\s*(?:static\s+)?(?:volatile\s+)?(?:const\s+)?(?:uint8_t|uint16_t|uint32_t|int8_t|int16_t|int32_t|bool|char|int|float|double)\s+(?:__xdata\s+)?[a-zA-Z_][a-zA-Z0-9_]*(?:\[[^\]]*\])?\s*(?:=.*)?;', line_stripped):
                    # Remove static keyword but keep the definition
                    var_def = line_stripped.replace('static ', '').replace('static\t', '')
                    global_vars.append(var_def)

        return global_vars

    def split_file(self, input_file: str, output_dir: str = None) -> None:
        """Split the C file into multiple files, one per function"""
        input_path = Path(input_file)

        if not input_path.exists():
            raise FileNotFoundError(f"Input file '{input_file}' not found")

        if output_dir is None:
            output_dir = input_path.stem + "_split"

        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)

        # Read the input file
        with open(input_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # Extract header content (includes, defines, but exclude globals to avoid duplication)
        header_content = self.extract_header_content(content, exclude_global_vars=True)

        # Extract functions (before removing static keywords)
        functions_with_static = self.extract_functions_preserve_static(content)
        functions = self.extract_functions(content)

        # Extract global variables
        global_variables = self.extract_global_variables(content)

        if not functions:
            print(f"No functions found in '{input_file}'")
            return

        print(f"Found {len(functions)} functions in '{input_file}':")

        # Create individual files for each function
        for i, (func_name, func_content, _, _) in enumerate(functions):
            output_file = output_path / f"{func_name}.c"

            with open(output_file, 'w', encoding='utf-8') as f:
                if header_content.strip():
                    f.write(header_content)
                    f.write('\n\n')

                # Include the generated function signatures header for cross-references
                f.write(f'#include "function_signatures.h"\n\n')

                # Define global variables only in the first split file to avoid duplicates
                if i == 0 and global_variables:
                    f.write("// Global variable definitions (defined only once)\n")
                    for var_def in global_variables:
                        f.write(f"{var_def}\n")
                    f.write('\n')

                f.write(func_content)
                f.write('\n')

            print(f"  - {func_name} -> {output_file}")

        # Create a summary file with function signatures for static functions only
        summary_file = output_path / "function_signatures.h"
        with open(summary_file, 'w', encoding='utf-8') as f:
            f.write(f"// Function declarations for static functions from {input_path.name}\n")
            f.write(f"// Only includes functions that were originally static\n\n")
            f.write(f"#ifndef {input_path.stem.upper()}_SPLIT_H\n")
            f.write(f"#define {input_path.stem.upper()}_SPLIT_H\n\n")

            # Add extern declarations for global variables so all split files can access them
            if global_variables:
                f.write("// Extern declarations for global variables\n")
                for var_def in global_variables:
                    # Convert to extern declaration
                    extern_line = var_def
                    if '=' in extern_line:
                        extern_line = extern_line.split('=')[0].strip() + ';'
                    if not extern_line.startswith('extern'):
                        extern_line = 'extern ' + extern_line
                    f.write(f"{extern_line}\n")
                f.write('\n')

            # Find and declare only static functions
            static_functions = []
            for func_name, func_content_with_static, _, _ in functions_with_static:
                # Check if this function was originally static
                first_line = func_content_with_static.split('\n')[0]
                if 'static' in first_line:
                    # Extract function signature without static keyword
                    lines = func_content_with_static.split('\n')
                    signature_lines = []

                    for line in lines:
                        if '{' in line:
                            brace_pos = line.find('{')
                            signature_lines.append(line[:brace_pos].rstrip())
                            break
                        signature_lines.append(line)

                    signature = '\n'.join(signature_lines).strip()
                    signature = signature.replace('static ', '').replace('static\t', '')
                    static_functions.append(f"{signature};")

            if static_functions:
                f.write("// Declarations for originally static functions\n")
                for sig in static_functions:
                    f.write(f"{sig}\n\n")
            else:
                f.write("// No static functions found\n\n")

            f.write(f"#endif // {input_path.stem.upper()}_SPLIT_H\n")

        print(f"\nFunction signatures saved to: {summary_file}")
        print(f"All files saved to directory: {output_path}")


def main():
    if len(sys.argv) < 2:
        print("Usage: python c_function_splitter.py <input_file.c> [output_directory]")
        sys.exit(1)

    input_file = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else None

    splitter = CFunctionSplitter()

    try:
        splitter.split_file(input_file, output_dir)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
