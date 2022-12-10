# Security Policy

## Supported Versions
The current version of Turb-OS isn't really stable. We don't recommand using it as a base for an other OS or as an example. 

## Reporting a Vulnerability

We once faced an issue: getting out of our VM filesystem. Let's take an example: 
You're using Turb-OS, you creating dirs, files and navigating in it. You miss type `cd ../` for `cd ..:` you may end-up getting out of Turb-OS filesystem and have your shell in Turb-OS looking like: 
```bash
root@~/Username/Turb-OS/system/.../: 
```

By this, understand that you may end-up going out of the VM. We created the problem during the development part but I couldn't recreate it now (november 2022).
