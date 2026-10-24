// Apenas para o emulador: usa o assinador do SDK com um certificado de teste.
// O WGT distribuído continua sem assinatura; TVs físicas usam o próprio DUID.
import java.nio.file.*;
import org.tizen.common.sign.preferences.SigningProfile;
import org.tizen.common.sign.preferences.SigningProfileItem;
import org.tizen.common.sign.signer.TizenIncrementalSigner;
public final class SignEmulatorWidget {
 public static void main(String[] args) throws Exception {
  if(args.length!=3)throw new IllegalArgumentException("SDK, autor e pasta WGT");
  org.apache.xml.security.Init.init();
  Path sdk=Paths.get(args[0]);
  Path certs=sdk.resolve("tools/certificate-generator/certificates/distributor");
  SigningProfile profile=new SigningProfile("StreamMaxEmulator");
  SigningProfileItem author=new SigningProfileItem(args[1],"author",true);
  author.setPassword("StreamMaxEmulator2026".toCharArray());
  SigningProfileItem distributor=new SigningProfileItem(certs.resolve("tizen-distributor-signer.p12").toString(),"distributor",false);
  distributor.setPassword("tizenpkcs12passfordsigner".toCharArray());
  distributor.setCAPath(certs.resolve("tizen-distributor-ca.cer").toString());
  profile.setProfileItem(0,author);profile.setProfileItem(1,distributor);
  if(!TizenIncrementalSigner.checkCertificatePassword(author.getKeyLocation(),"StreamMaxEmulator2026"))throw new IllegalStateException("Certificado de teste inválido");
  new TizenIncrementalSigner(profile,args[2]).sign(false);
  for(String name:new String[]{"author-signature.xml","signature1.xml"})
   if(Files.size(Paths.get(args[2],name))<100)throw new IllegalStateException("Assinatura não gerada: "+name);
  System.out.println("Assinaturas do emulador geradas pelo SDK Tizen.");
 }
}
