# Wildcard support for HelenOS - MSoC Project by Patrik Pritrsky  

In this project, I added support for wildcards in the HelenOS shell.  
So, if the user wants to delete all text files in a directory, they just  
need to type 'rm *.txt', instead of listing them all one by one.  

## Features  

### Standard wildcard *  

Expands to zero or more characters, evaluation happens recursively at  
all levels where it occurs.  

For example, 'folder*/file*.txt' will find all text files starting with  
'file', in all subdirectories of the current directory that start with  
'folder'.  

### Recursive wildcard **  

Used to find files at arbitrary depth.  

For example, '**/*.txt' will find all text files, at any depth within  
the current directory.  

## List of changes to HelenOS  

- Added automated tests for wildcards  
- Created a function to detect whether a string contains a wildcard  
- Created a function to check whether a wildcard pattern matches a  
  file/directory name  
- Created a function for recursive expansion and finding all  
  occurrences of files/directories that match a path/filename containing  
  wildcards  
- Modified the HelenOS shell tokenizer to support wildcard expansion