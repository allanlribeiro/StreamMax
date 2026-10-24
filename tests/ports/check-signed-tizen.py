"""Confere o conteúdo sem acessar senhas ou chaves privadas do signatário."""
import hashlib,re,sys,zipfile
with zipfile.ZipFile(sys.argv[1]) as original,zipfile.ZipFile(sys.argv[2]) as signed:
 expected=set(original.namelist())
 signatures={n for n in signed.namelist() if re.fullmatch(r'(author-signature|signature\d+)\.xml',n)}
 if not {'author-signature.xml','signature1.xml'}<=signatures:raise SystemExit('Assinaturas ausentes')
 actual={n for n in signed.namelist() if not n.endswith('/')} - signatures
 if actual!=expected:raise SystemExit('O WGT assinado contém arquivos diferentes do original')
 for name in expected:
  if hashlib.sha256(original.read(name)).digest()!=hashlib.sha256(signed.read(name)).digest():raise SystemExit('Conteúdo diferente: '+name)
print('Conteúdo do WGT assinado corresponde ao pacote compilado.')
