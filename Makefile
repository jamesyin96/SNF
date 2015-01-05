target: clientapp serverSNFS
.PHONY : target

clientapp: clientapp.c clientSNFS.c
	gcc -o clientapp -g clientapp.c clientSNFS.c

serverSNFS: serverSNFS.c
	gcc -o serverSNFS -g serverSNFS.c