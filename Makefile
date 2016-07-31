.SUFFIXES: .cpp .hpp																										  
# Programs
SHELL 	= bash
CC     	= g++
LD	= ld
RM 	= rm
ECHO	= /bin/echo
CAT	= cat
PRINTF	= printf
SED	= sed	
DOXYGEN = doxygen	
PDFLATEX = pdflatex
BIBTEX = bibtex	
######################################																			  
# Project Name (generate executable with this name)		
PDF = report		
BEAMER = Presentation	
# Project Paths							
PROJECT_ROOT=./

SRCDIR = $(PROJECT_ROOT)/src
OBJDIR = $(PROJECT_ROOT)/obj
BINDIR = $(PROJECT_ROOT)
DOCDIR = $(PROJECT_ROOT)/doc

# Library Paths			

#Libraries																						  
LIBS := -lpthread -lcrypt

# Compiler and Linker flags
CPPFLAGS =-g -O3 -pg -std=c++11 -lcrypt -pthread
######################################																			  
NO_COLOR=\e[0m					
OK_COLOR=\e[1;32m								
ERR_COLOR=\e[1;31m						
WARN_COLOR=\e[1;33m
MESG_COLOR=\e[1;34m		
FILE_COLOR=\e[1;37m				

OK_STRING="[OK]"
ERR_STRING="[ERRORS]"
WARN_STRING="[WARNINGS]"
OK_FMT="${OK_COLOR}%30s\n${NO_COLOR}"
ERR_FMT="${ERR_COLOR}%30s\n${NO_COLOR}"
WARN_FMT="${WARN_COLOR}%30s\n${NO_COLOR}"
######################################
SRCS := $(wildcard $(SRCDIR)/*.cpp)
INCS := $(wildcard $(SRCDIR)/*.hpp)
OBJS := $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

.PHONY: all setup clean distclean 		

all : setup server user worker

setup : 
	@mkdir -p obj
	
server user worker: server.o user.o worker.o	
	@$(PRINTF) "$(MESG_COLOR)Building executable:$(NO_COLOR) $(FILE_COLOR) %25s$(NO_COLOR)" "$(notdir $@)" 
	@$(CC) -o "$(BINDIR)/$@" $(LDFLAGS) "$(OBJDIR)/$@.o" $(LIBS) 2> temp.log || touch temp.err
	@if test -e temp.err; \
	then $(PRINTF) $(ERR_FMT) $(ERR_STRING) && $(CAT) temp.log; \
	elif test -s temp.log; \
	then $(PRINTF) $(WARN_FMT) $(WARN_STRING) && $(CAT) temp.log; \
	else $(PRINTF) $(OK_FMT) $(OK_STRING);\
	fi;
	@$(RM) -f temp.log temp.err

server.o :
	@$(PRINTF) "$(MESG_COLOR)Compiling: $(NO_COLOR) $(FILE_COLOR) %16s$(NO_COLOR)" "$(notdir $@)" 
	@$(CC) $(CPPFLAGS) -c "$(SRCDIR)/server.cpp" -o "$(OBJDIR)/$@" -MD 2> temp.log || touch temp.err
	@if test -e temp.err; \
	then $(PRINTF) $(ERR_FMT) $(ERR_STRING) && $(CAT) temp.log; \
	elif test -s temp.log; \
	then $(PRINTF) $(WARN_FMT) $(WARN_STRING) && $(CAT) temp.log; \
	else $(PRINTF) $(OK_FMT) $(OK_STRING);\
	fi;
	@$(RM) -f temp.log temp.err
	
user.o   :
	@$(PRINTF) "$(MESG_COLOR)Compiling: $(NO_COLOR) $(FILE_COLOR) %16s$(NO_COLOR)" "$(notdir $@)" 
	@$(CC) $(CPPFLAGS) -c "$(SRCDIR)/user.cpp" -o "$(OBJDIR)/$@" -MD 2> temp.log || touch temp.err
	@if test -e temp.err; \
	then $(PRINTF) $(ERR_FMT) $(ERR_STRING) && $(CAT) temp.log; \
	elif test -s temp.log; \
	then $(PRINTF) $(WARN_FMT) $(WARN_STRING) && $(CAT) temp.log; \
	else $(PRINTF) $(OK_FMT) $(OK_STRING);\
	fi;
	@$(RM) -f temp.log temp.err 

worker.o :
	@$(PRINTF) "$(MESG_COLOR)Compiling: $(NO_COLOR) $(FILE_COLOR) %16s$(NO_COLOR)" "$(notdir $@)" 
	@$(CC) $(CPPFLAGS) -c "$(SRCDIR)/worker.cpp" -o "$(OBJDIR)/$@" -MD 2> temp.log || touch temp.err
	@if test -e temp.err; \
	then $(PRINTF) $(ERR_FMT) $(ERR_STRING) && $(CAT) temp.log; \
	elif test -s temp.log; \
	then $(PRINTF) $(WARN_FMT) $(WARN_STRING) && $(CAT) temp.log; \
	else $(PRINTF) $(OK_FMT) $(OK_STRING);\
	fi;
	@$(RM) -f temp.log temp.err

clean:																							  
	@$(ECHO) -n "Cleaning up..."
	@$(RM) -rf $(OBJDIR) $(SRCDIR)/*~ temp.log temp.err 	  
	@$(ECHO) "Done"

distclean: clean
	@$(RM) -rf $(BINDIR)/server $(BINDIR)/worker $(BINDIR)/user  
