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
            r'^([a-zA-Z_][a-zA-Z0-9_\s\*]*)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*(?:__[a-zA-Z_]+\s*\([^)]*\))?\s*\{',
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
            functions.append((func_name, func_content, start_pos, end_brace + 1))
        
        return functions

    def extract_header_content(self, content: str) -> str:
        """Extract includes, defines, typedefs, and global variables"""
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
        
        # Extract global variables (before any function)
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
                not 'typedef' in line):
                
                # Simple heuristic for global variables
                if re.match(r'^\s*(?:static\s+)?(?:volatile\s+)?(?:const\s+)?[a-zA-Z_]', line):
                    global_vars.append(line)
        
        if global_vars:
            header_parts.extend(global_vars)
            header_parts.append("")  # Empty line after global vars
        
        return '\n'.join(header_parts)

    def split_file(self, input_file: str, output_dir: str = None) -> None:
        """Split the C file into multiple files, one per function"""
        input_path = Path(input_file)
        
        if not input_path.exists():
            raise FileNotFoundError(f"Input file '{input_file}' not found")
        
        if output_dir is None:
            output_dir = input_path.stem + "_split"
        
        output_path = Path(output_dir)
        output_path.mkdir(exist_ok=True)
        
        # Read the input file
        with open(input_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Extract header content (includes, defines, globals)
        header_content = self.extract_header_content(content)
        
        # Extract functions
        functions = self.extract_functions(content)
        
        if not functions:
            print(f"No functions found in '{input_file}'")
            return
        
        print(f"Found {len(functions)} functions in '{input_file}':")
        
        # Create individual files for each function
        for func_name, func_content, _, _ in functions:
            output_file = output_path / f"{func_name}.c"
            
            with open(output_file, 'w', encoding='utf-8') as f:
                if header_content.strip():
                    f.write(header_content)
                    f.write('\n\n')
                f.write(func_content)
                f.write('\n')
            
            print(f"  - {func_name} -> {output_file}")
        
        # Create a summary file with all function signatures
        summary_file = output_path / "function_signatures.h"
        with open(summary_file, 'w', encoding='utf-8') as f:
            f.write(f"// Function signatures extracted from {input_path.name}\n\n")
            if header_content.strip():
                f.write(header_content)
                f.write('\n\n')
            
            for func_name, func_content, _, _ in functions:
                # Extract just the function signature
                lines = func_content.split('\n')
                signature_lines = []
                brace_found = False
                
                for line in lines:
                    if '{' in line:
                        # Include the line up to the opening brace
                        brace_pos = line.find('{')
                        signature_lines.append(line[:brace_pos].rstrip())
                        break
                    signature_lines.append(line)
                
                signature = '\n'.join(signature_lines).strip()
                f.write(f"{signature};\n\n")
        
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
