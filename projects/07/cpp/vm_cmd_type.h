#ifndef VM_CMD_TYPES_ENUMS
#define VM_CMD_TYPES_ENUMS

enum vmcmdtypesforhack{C_INVALID, C_COMMENT, C_EMPTY, C_ARITHMETIC, C_PUSH, C_POP, C_LABEL, C_GOTO, C_IF, C_FUNCTION, C_RETURN, C_CALL};

typedef enum vmcmdtypesforhack vm_command_type;

std::string enumlist[]={"C_INVALID", "C_COMMENT", "C_EMPTY", "C_ARITHMETIC", "C_PUSH", "C_POP", "C_LABEL", "C_GOTO",
			"C_IF", "C_FUNCTION", "C_RETURN", "C_CALL"};

#endif
