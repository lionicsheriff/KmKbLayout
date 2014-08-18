if not exist bin\kblayout.cer (
	makecert -r -pe -ss PrivateCertStore -n "CN=kblayout" bin\kblayout.cer
)
signtool sign /v /s PrivateCertStore /n kblayout /t http://timestamp.verisign.com/scripts/timestamp.dll "bin\amd64\kblayout.sys"