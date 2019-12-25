Documentation for Kernel Assignment 1
=====================================

+------------------------+
| BUILD & RUN (Required) |
+------------------------+

Replace "(Comments?)" with the command the grader should use to compile your kernel (should simply be "make").
    To compile the kernel, the grader should type: ./weenix -n
If you have additional instruction for the grader, replace "(Comments?)" with your instruction (or with the word "none" if you don't have additional instructions):
    Additional instructions for building/running this assignment: When DRIVERS=0, make CS402TESTS=8 or less, because the 9th Faber Test requires separate process, which is ensured in DRIVERS=1.

+-------------------------+
| SELF-GRADING (Required) |
+-------------------------+

Replace each "(Comments?)" with appropriate information and each stand-alone "?"
with a numeric value:

(A.1) In main/kmain.c:
    (a) In bootstrap(): 3 out of 3 pts
    (b) In initproc_create(): 3 out of 3 pts

(A.2) In proc/proc.c:
    (a) In proc_create(): 4 out of 4 pts
    (b) In proc_cleanup(): 7 out of 7 pts
    (c) In do_waitpid(): 6 out of 6 pts

(A.3) In proc/kthread.c:
    (a) In kthread_create(): 2 out of 2 pts
    (b) In kthread_cancel(): 1 out of 1 pt
    (c) In kthread_exit(): 3 out of 3 pts

(A.4) In proc/sched_helper.c:
    (a) In sched_wakeup_on(): 1 out of 1 pt

(A.5) In proc/sched.c:
    (a) In sched_make_runnable(): 1 out of 1 pt

(A.6) In proc/kmutex.c:
    (a) In kmutex_lock(): 1 out of 1 pt
    (b) In kmutex_lock_cancellable(): 1 out of 1 pt
    (c) In kmutex_unlock(): 2 out of 2 pts

(B) Kshell : 20 out of 20 pts
    What is your kshell command to invoke faber_thread_test(): faber
    What is your kshell command to invoke sunghan_test(): sunghan
    What is your kshell command to invoke sunghan_deadlock_test(): deadlock

(C.1) waitpid any test, etc. (4 out of 4 pts)
(C.2) Context switch test (1 out of 1 pt)
(C.3) wake me test, etc. (2 out of 2 pts)
(C.4) wake me uncancellable test, etc. (2 out of 2 pts)
(C.5) cancel me test, etc. (4 out of 4 pts)
(C.6) reparenting test, etc. (2 out of 2 pts)
(C.7) show race test, etc. (3 out of 3 pts)
(C.8) kill child procs test (2 out of 2 pts)
(C.9) proc kill all test (2 out of 2 pts)

(D.1) sunghan_test(): producer/consumer test (9 out of 9 pts)
(D.2) sunghan_deadlock_test(): deadlock test (4 out of 4 pts)

(E) Additional self-checks: (10 out of 10 pts)
    Please provide details, add subsections and/or items as needed; or, say that "none is needed".
    Details: none is needed

Missing/incomplete required section(s) in README file (procs-README.txt): (-0 pts)
Submitted binary file : (-0 pts)
Submitted extra (unmodified) file : (-0 pts)
Wrong file location in submission : (-0 pts)
Extra printout when running with DBG=error,test in Config.mk : (-0 pts)
Incorrectly formatted or mis-labeled "conforming dbg() calls" : (-0 pts)
Cannot compile : (-0 pts)
Compiler warnings : (-0 pts)
"make clean" : (-0 pts)
Kernel panic : (-0 pts)
Kernel freezes : (-0 pts)
Cannot halt kernel cleanly : (-0 pts)

+--------------------------------------+
| CONTRIBUTION FROM MEMBERS (Required) |
+--------------------------------------+

1)  Names and USC e-mail addresses of team members: 
    Suyash Sardar: ssardar@usc.edu
    Pooja Mahadev Soundalgekar: mahadevs@usc.edu
    Prajith Gurudath: prajithg@usc.edu
    Bhanu Kiran Atturu: atturu@usc.edu
2)  Is the following statement correct about your submission (please replace
        "(Comments?)" with either "yes" or "no", and if the answer is "no",
        please list percentages for each team member)?  "Each team member
        contributed equally in this assignment": yes

+---------------------------------+
| BUGS / TESTS TO SKIP (Required) |
+---------------------------------+

Are there are any tests mentioned in the grading guidelines test suite that you
know that it's not working and you don't want the grader to run it at all so you
won't get extra deductions, please replace "(Comments?)" below with your list.
(Of course, if the grader won't run such tests in the plus points section, you
will not get plus points for them; if the garder won't run such tests in the
minus points section, you will lose all the points there.)  If there's nothing
the grader should skip, please replace "(Comments?)" with "none".

Please skip the following tests: none

+--------------------------------------------------------------------------------------------+
| ADDITIONAL INFORMATION FOR GRADER (Optional, but the grader should real what you add here) |
+--------------------------------------------------------------------------------------------+

+-----------------------------------------------+
| OTHER (Optional) - Not considered for grading |
+-----------------------------------------------+

Comments on design decisions: none
