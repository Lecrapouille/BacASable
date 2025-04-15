# Ansible Hello World Project

This is a simple Ansible project that demonstrates basic automation tasks. It creates a directory, copies a Python script, and executes it.

## Playbook Overview

The project performs three main tasks:

- Creates a directory in /tmp with specified folder name and permissions.
- Copies a simple Python script that prints "Hello World".
- Executes the Python script.

The Python script (`roles/hello_world/files/hello.py`) outputs:

```bash
Hello World from Python!
This script was created by Ansible
```

You can do `ls -la /tmp` to see created folder `my_ansible_project` which shall contains the Python script.

## Execution

```bash
# Run the playbook
ansible-playbook -i inventory.ini playbooks/hello_world.yml
```

## Project Structure

```txt
.
├── ansible.cfg         # Global Ansible configuration
├── inventory.ini       # Host inventory
├── playbooks/          # Ansible playbooks
└── roles/              # Reusable Ansible roles
```

### Inventory Configuration

The current inventory file `inventory.ini` is configured for local execution.
Replace `qq` with your desired username.

### Roles

Roles are organized in the `roles/` directory and can be reused across different playbooks.
This project includes a single role named `hello_world` which installs and executes the Python script. Default variables are defined in `roles/hello_world/defaults/main.yml`.

### Playbooks

Playbooks are stored in the `playbooks/` directory and define tasks to be executed on hosts.
Default variables are overridden in this playbook.

## Syntax Checking

```bash
# Check playbook syntax
ansible-playbook playbooks/hello_world.yml --syntax-check

# Verify inventory
ansible-inventory -i inventory.ini --list

# Check YAML syntax
find . -name "*.yml" -o -name "*.yaml" | xargs -I {} yamllint {}
```

## Prerequisites

- Ansible 2.9 or higher
- Python 3.6 or higher
- SSH access to target hosts (if applicable)
