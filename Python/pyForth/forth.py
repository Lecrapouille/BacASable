#!/usr/bin/env python3

import sys, readline
from typing import Callable, Optional

# Configuration de readline (permet de gérer les flèches, l'historique, l'auto-complétion, etc.)
readline.parse_and_bind('tab: complete')
readline.parse_and_bind('set editing-mode emacs')

class DictionaryEntry:
    """
    \brief Represents an entry in the Forth dictionary
    A dictionary entry contains:
    - Link to previous word
    - Name of the word
    - Flags (immediate, etc)
    - Code or list of words to execute
    """
    def __init__(self, name: str, immediate: bool = False, prev_entry: Optional['DictionaryEntry'] = None):
        self.name = name.upper()        # Word name
        self.immediate = immediate      # True if word should be executed during compilation
        self.prev_entry = prev_entry    # Link to previous dictionary entry
        self.code = []                  # List of words to execute or function for builtins
        self.code_field_address = 0     # Address of the code field in memory
        
    def __call__(self, vm):
        if callable(self.code):
            self.code(vm)  # For builtins
        else:
            # Save instruction pointer on return stack
            ip = 0
            vm.rpush(ip)
            
            while ip < len(self.code):
                # Get next instruction
                word = self.code[ip]
                
                # If it's a number, push it directly
                if isinstance(word, (int, float)):
                    vm.push(word)
                    ip += 1
                else:
                    # Execute the word
                    word(vm)
                    ip += 1
            
            # Restore instruction pointer
            vm.rpop()

class Word(DictionaryEntry):
    """
    \brief Represents a Forth word compiled by the user
    Example:
    : square dup * ;
    square 4 .
    """
    def __init__(self, name: str, immediate: bool = False, prev_entry: Optional[DictionaryEntry] = None):
        super().__init__(name, immediate, prev_entry)

class Builtin(DictionaryEntry):
    """
    \brief Represents a built-in Forth word (e.g. +, -, *, DUP, DROP, SWAP ...)
    """
    def __init__(self, name: str, fn: Callable, immediate: bool = False, prev_entry: Optional[DictionaryEntry] = None):
        super().__init__(name, immediate, prev_entry)
        self.code = fn  # La fonction est directement le code à exécuter

class ForthVM:
    """
    \brief Simple Forth interpreter with data stack, return stack and memory
    """
    def __init__(self):
        self.data_stack = []              # Stack for data manipulation
        self.return_stack = []            # Return stack for nested calls
        self.memory = bytearray(65536)    # Memory space (64K)
        self.here = 0                     # Current allocation pointer
        self.latest = None                # Pointer to latest word in dictionary
        self.compiling = False            # Compilation or interpretation mode flag
        self.current_definition = None    # Current word being compiled
        self.input_source = sys.stdin     # Current input stream (read Forth code)
        self.input_buffer = ""            # Input buffer for interactive mode
        self.buffer_pos = 0               # Current position in input buffer
        
        # Initialize built-in words
        self.init_builtins()
    
    def init_builtins(self):
        """Initialize all built-in Forth words"""
        # Stack operations
        self.add_builtin('+', lambda vm: vm.push(vm.pop() + vm.pop()))
        self.add_builtin('-', lambda vm: vm.push(-vm.pop() + vm.pop()))
        self.add_builtin('*', lambda vm: vm.push(vm.pop() * vm.pop()))
        self.add_builtin('DUP', lambda vm: vm.push(vm.peek()))
        self.add_builtin('DROP', lambda vm: vm.pop())
        self.add_builtin('SWAP', lambda vm: vm.swap())
        self.add_builtin('.', lambda vm: print(vm.pop(), end=' '))
        self.add_builtin('.S', lambda vm: vm.show_stack())
        
        # Memory operations
        self.add_builtin('!', lambda vm: vm.store())
        self.add_builtin('@', lambda vm: vm.fetch())
        self.add_builtin('HERE', lambda vm: vm.push(vm.here))
        self.add_builtin('VARIABLE', lambda vm: vm.create_variable())
        
        # Input stream operations
        self.add_builtin('WORD', lambda vm: vm.word())
        
        # Compilation words
        self.add_builtin(':', lambda vm: vm.begin_compile(), immediate=True)
        self.add_builtin(';', lambda vm: vm.end_compile(), immediate=True)
        self.add_builtin('IMMEDIATE', lambda vm: vm.make_immediate(), immediate=True)
        
        # Dictionary inspection
        self.add_builtin('WORDS', lambda vm: vm.list_words())
        
        # Literal handling
        self.add_builtin('LITERAL', lambda vm: vm.push(next(vm.tokens)), immediate=True)
    
    def add_builtin(self, name: str, fn: Callable, immediate: bool = False):
        """
        \brief Add a built-in word to the dictionary
        \param [in] name Word name
        \param [in] fn Function to execute
        \param [in] immediate True if word should be executed during compilation
        """
        word = Builtin(name, fn, immediate, self.latest)
        word.code_field_address = self.here
        self.latest = word
        self.here += 8  # Simulated size of a code field (pointer)

    def find_word(self, name: str) -> Optional[DictionaryEntry]:
        """
        \brief Find a word in the dictionary
        \param [in] name Word name to find
        \return The word entry or None if not found
        """
        current = self.latest
        while current is not None:
            if current.name == name:
                return current
            current = current.prev_entry
        return None
    
    def word(self):
        """
        \brief Advance in input stream until delimiter is found
        \details The WORD primitive expects an ASCII delimiter on the stack
        """
        delimiter = chr(self.pop())
        
        # Skip leading delimiters
        while True:
            c = self.input_source.read(1)
            if not c:  # EOF
                return
            if c != delimiter:
                break
                
        # Read until delimiter
        while True:
            c = self.input_source.read(1)
            if not c or c == delimiter:  # EOF or delimiter found
                break

    def store(self):
        """Store a value in memory: addr n !"""
        value = self.pop()
        addr = self.pop()
        if 0 <= addr < len(self.memory) - 7:  # 8 bytes for numbers
            for i in range(8):
                self.memory[addr + i] = (value >> (i * 8)) & 0xFF
        else:
            raise ValueError(f"Adresse mémoire invalide: {addr}")
    
    def fetch(self):
        """Fetch a value from memory: addr @"""
        addr = self.pop()
        if 0 <= addr < len(self.memory) - 7:
            value = 0
            for i in range(8):
                value |= self.memory[addr + i] << (i * 8)
            self.push(value)
        else:
            raise ValueError(f"Adresse mémoire invalide: {addr}")
    
    def create_variable(self, name=None):
        """Create a new variable"""
        if self.compiling:
            name = self.pop()
        else:
            name = next(self.tokens)
        
        # Allouer l'espace pour la variable
        var_addr = self.here
        self.here += 8  # 8 bytes pour un nombre
        
        # Créer le mot qui pousse l'adresse sur la pile
        word = Word(name, prev_entry=self.latest)
        word.code = [lambda vm=self, addr=var_addr: vm.push(addr)]
        word.code_field_address = var_addr
        self.latest = word
    
    def push(self, value):
        """Push a value onto the data stack"""
        self.data_stack.append(value)
    
    def pop(self):
        """Pop a value from the data stack"""
        if not self.data_stack:
            raise IndexError("Stack underflow")
        return self.data_stack.pop()
    
    def peek(self):
        """Look at the top value without removing it"""
        if not self.data_stack:
            raise IndexError("Stack empty")
        return self.data_stack[-1]
    
    def swap(self):
        """Swap the top two values on the stack"""
        a = self.pop()
        b = self.pop()
        self.push(a)
        self.push(b)
    
    def show_stack(self):
        """Display the current stack contents"""
        print(f"<{len(self.data_stack)}> ", end='')
        print(self.data_stack)
    
    def begin_compile(self):
        """Start compiling a new word"""
        self.compiling = True
        name = next(self.tokens)
        self.current_definition = Word(name, prev_entry=self.latest)
        self.current_definition.code_field_address = self.here
        self.here += 8  # Espace pour le code field
    
    def end_compile(self):
        """Finish compiling the current word"""
        if not self.compiling:
            raise RuntimeError("Pas en compilation")
        self.compiling = False
        self.latest = self.current_definition
        self.current_definition = None
    
    def compile(self, token):
        """Compile a token into the current definition"""
        word = self.find_word(token)
        if word:
            if word.immediate:
                word(self)
            else:
                self.current_definition.code.append(word)
        else:
            try:
                number = int(token)
                # Compile LITERAL followed by the number
                literal_word = self.find_word('LITERAL')
                self.current_definition.code.append(literal_word)
                self.current_definition.code.append(number)
            except ValueError:
                raise ValueError(f"Mot inconnu pendant la compilation: {token}")

    def interpret(self, word: str):
        """
        \brief Interpret a single Forth word
        \param [in] word Word to interpret
        """
        word_entry = self.find_word(word)
        # Si le mot est trouvé, l'exécuter.
        if word_entry:
            word_entry(self)
        # Sinon, essayer de le convertir en nombre.
        else:
            try:
                # Le token est un nombre, le mettre dans la pile.
                self.push(int(word))
            except ValueError:
                # Ce n'est pas un mot connu, ni un nombre => mot inconnu du dictionnaire.
                raise ValueError(f"Mot inconnu: {word}")

    def next_word(self) -> Optional[str]:
        """
        \brief Lit le prochain mot Forth (caractères jusqu'au prochain espace/tab/retour chariot)
        \return Le mot lu ou None si fin d'entrée
        """
        # Si buffer vide ou position à la fin, lire une nouvelle ligne
        if not self.input_buffer or self.buffer_pos >= len(self.input_buffer):
            if self.input_source.isatty():
                try:
                    self.input_buffer = input() + '\n'
                    self.buffer_pos = 0
                except EOFError:
                    return None
            else:
                self.input_buffer = self.input_source.readline()
                self.buffer_pos = 0
                if not self.input_buffer:  # EOF
                    return None

        # Ignorer les espaces au début
        while self.buffer_pos < len(self.input_buffer) and self.input_buffer[self.buffer_pos].isspace():
            self.buffer_pos += 1

        if self.buffer_pos >= len(self.input_buffer):
            self.input_buffer = ""  # Reset buffer pour la prochaine ligne
            return None

        # Lire jusqu'au prochain espace
        start = self.buffer_pos
        while self.buffer_pos < len(self.input_buffer) and not self.input_buffer[self.buffer_pos].isspace():
            self.buffer_pos += 1

        return self.input_buffer[start:self.buffer_pos]

    def process(self, input_source):
        """
        \brief Process Forth code from a source, handling both compilation and interpretation
        \param [in] input_source File object to read from
        """
        old_source = self.input_source
        self.input_source = input_source
        
        try:
            while True:
                word = self.next_word()
                # Fin de ligne ou EOF
                if word is None:
                    break
                # Si en mode compilation, compiler le mot
                if self.compiling:
                    self.compile(word)
                # Sinon, interpréter le mot
                else:
                    self.interpret(word)
        finally:
            self.input_source = old_source

    def load_file(self, filename: str):
        """
        \brief Load and process a Forth source file
        \param [in] filename Path to the Forth source file
        """
        try:
            with open(filename, 'r') as f:
                self.process(f)
        except FileNotFoundError:
            raise RuntimeError(f"Fichier non trouvé: {filename}")
        except Exception as e:
            raise RuntimeError(f"Erreur lors de la lecture du fichier {filename}: {str(e)}")

    def make_immediate(self):
        """
        \brief Mark the most recently defined word as immediate
        The word will be executed during compilation instead of being compiled
        """
        if self.latest is None:
            raise RuntimeError("Aucun mot à marquer comme immédiat")
        self.latest.immediate = True

    def list_words(self):
        """Display all defined words in the dictionary"""
        print("\nMots disponibles:")
        line_length = 0
        current = self.latest
        
        while current is not None:
            word = current.name
            if line_length + len(word) + 1 > 60:  # Wrap at 60 characters
                print()
                line_length = 0
            print(word, end=' ')
            line_length += len(word) + 1
            current = current.prev_entry
        print("\n")

    def rpush(self, value):
        """
        \brief Push a value onto the return stack
        \param [in] value Value to push
        """
        self.return_stack.append(value)
    
    def rpop(self):
        """
        \brief Pop a value from the return stack
        \return The popped value
        """
        if not self.return_stack:
            raise IndexError("Return stack underflow")
        return self.return_stack.pop()
    
    def rpeek(self):
        """
        \brief Look at the top value of the return stack without removing it
        \return The top value
        """
        if not self.return_stack:
            raise IndexError("Return stack empty")
        return self.return_stack[-1]

def main():
    """Main REPL loop"""
    forth = ForthVM()
    print("Interpréteur Forth simple")
    print("Tapez 'bye' pour quitter")
    print("Tapez '.S' pour voir la pile")
    print("Ctrl-D pour quitter")
    
    # Traiter les arguments de la ligne de commande
    if len(sys.argv) > 1:
        for filename in sys.argv[1:]:
            try:
                forth.load_file(filename)
            except Exception as e:
                print(f"Erreur: {e}")
                sys.exit(1)
    
    # Mode interactif
    while True:
        try:
            forth.process(sys.stdin)
        except KeyboardInterrupt:  # Ctrl-C
            print("\nAu revoir!")
            break
        except Exception as e:
            print(f"Erreur: {e}")

if __name__ == "__main__":
    main() 