# CSOPESY

Welcome to our CSOPESY OS Emulator! This CLI-based emulator simulates CPU scheduling and process management. 

To manually compile the OS emulator via terminal: g++ menu_screen.cpp Utilities.cpp Console.cpp ProcessManager.cpp Process.cpp ConsoleManager.cpp FCFS.cpp -o main.exe

The user can edit the **config.txt** file to suit your simulation needs. For example:

num-cpu 10
scheduler rr
quantum-cycles 5
batch-process-freq 3000
min-ins 100
max-ins 500
delay-per-exec 0

**num-cpu** - number of CPU cores (threads) to simulate
**scheduler** - scheduler type (rr, fcfs)
**quantum-cycles** - number of cycles per quantum for RR
**batch-process-freq** - frequency (ms) of auto process creation
**delay-per-exec** - delay (ms) per instruction execution (0 = no delay)

Once you launch the program, you will land on the **Menu Screen**. You have to enter the **initialize** command to load the configuration from **config.txt**. 

**scheduler-start** - automatically generates and assigns processes to CPUs.
**scheduler-stop** - stops the generation of new processes, but existing ones continue running.
**screen -s <process_name>** - creates a new process with its own set of instructions.
**screen -r <process_name>** - reopens the screen of an existing process where you can enter the following commands to interact with it:
            <li>
                  **process-smi** - view the current instruction list
                  **exit** - return to main menu
            <li>
**screen-ls** - lists running and finished processes.
**report-util** - generates a log file with the same process details shown in **screen -ls**.
**marquee** - switch to marquee console. **exit** to go back to the main menu.
**exit** - use this from the main menu to shut down the emulator gracefully.

## Extra Notes
Make sure to always run **initialize** first after launching otherwise the emulator wouldn't recognize any other commands


