---
name: Bug report
about: Create a report to help squash bugs
title: ''
labels: ''
assignees: ''

---

**1. Basic information**
***1.1 System information***
- **Terminal/Shell:** e.g.: Konsole 21.04 using BASH 5.1.8
- **Distro:**
- **Kernel:**

***1.2 cTune version***
run `ctune --version` to get a print out of all that info

**2. Describe the bug**
What seems to break and where (a screenshot for UI bugs would be useful).

**3. To Reproduce**
Steps to reproduce the behavior

**4. Expected behavior**
A clear and concise description of what you expected to happen.

**5. Screenshots (UI)**
If applicable, add screenshots to help explain your problem.

**6. Configuration**
The content of `ctune.cfg` when applicable.

**7. cTune log**
The content of the error log (run `ctune --debug` to generate more granular and useful info during execution).

**8. System log**
Copy of the system log's (syslog) cTune runtime specific entries where the bug occurred (output of `journalctl --utc -b -0 | grep ctune` if you're using systemd)

**9. Additional context**
Add any other context about the problem here.
