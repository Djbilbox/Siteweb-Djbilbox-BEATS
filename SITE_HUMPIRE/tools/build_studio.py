# Builds the HUMPIRE RECORDS studio page in every site language.
# Run: python tools/build_studio.py   (from SITE_HUMPIRE)
# Output: studio.html (EN) + fr/ es/ pt/ de/ studio.html
import os, html

BASE = 'https://humpireplugins.netlify.app'
LANGS = ['en', 'fr', 'es', 'pt', 'de']
LABEL = {'en': 'EN', 'fr': 'FR', 'es': 'ES', 'pt': 'PT', 'de': 'DE'}

def path(lang):
    return 'studio.html' if lang == 'en' else f'{lang}/studio.html'

# ---- prices (same in every language) ----
P = dict(single='220', beat='450', ep='850', empire='1 490')

T = {
'en': dict(
  title='HUMPIRE RECORDS — Recording Studio, Mix & Master',
  desc='HUMPIRE RECORDS: recording studio by DJBILBOX BEATS. Record, mix and master your single, EP or album with a producer behind the desk. Fixed-price packs from €220.',
  eyebrow='Recording Studio · by DJBILBOX BEATS', tag='Your voice. Our sound.', tag2='Built to be released.',
  lead='Record, mix and master your track with a producer behind the desk — the same hands behind 800+ beats and the HUMPIRE plugins. Fixed prices, no surprises, a finished record when you leave.',
  cta1='See the packs', cta2='Book a session',
  pil=[('Recording','Treated vocal booth, studio condenser mic, coaching on every take.'),('Mix & Master','Loud, clean, radio-ready — a sound that holds up on Spotify and in the car.'),('Custom beats','An original instrumental made for you — G-Funk, Oriental, Afro, Trap, Drill.'),('Artistic direction','Topline, structure, flow, cover art and a plan to release it properly.')],
  room_e='The room', room_h='Inside HUMPIRE Records', room_s='LED lights, a treated booth and a lounge to vibe between takes.',
  g=['The control room','The booth','The mic','The lounge'], galt=['Control room with LED lights and the HUMPIRE RECORDS logo on the wall screen','Artist recording vocals in the LED booth','Microphone in the LED vocal booth','Studio lounge with neon LED lights'],
  pr_e='Pricing', pr_h='Studio packs', pr_s='Fixed prices, everything included. Pay in full or in several instalments.',
  most='Most chosen', book='Book',
  packs=[('Single','One track, ready to drop',['2 h recording with engineer','Mix & master of 1 track','WAV + MP3 export','Artistic advice on the take']),
         ('Single + Beat','Your own instrumental',['Custom beat by DJBILBOX — exclusive','2 h recording with engineer','Mix & master of 1 track','WAV + MP3 + beat stems']),
         ('EP','5 tracks, one coherent project',['10 h recording with engineer','5 mixes + 5 masters','WAV + MP3 exports','Tracklist & artistic direction']),
         ('Empire','Album / full artist project',['10 tracks, 20 h recording','10 mixes + 10 masters','Cover art included','Backstage content for socials','Release plan & distribution support'])],
  ca_e='À la carte', ca_h='Single services',
  carte=[('Recording session with engineer','40 € / h'),('Block of 4 hours','150 €'),('Night session (22:00 – 06:00), per hour','50 € / h'),('Mix of 1 track','90 €'),('Master of 1 track','35 €'),('Mix + master of 1 track (remote, send your stems)','110 €'),('Custom exclusive beat','200 €'),('Cover art','60 €'),('Photo shoot · music video','On quote')],
  note='Mix & master is also available remotely, worldwide — send your stems, get your track back.',
  st_e='The process', st_h='From idea to release',
  steps=[('We talk','Send your project, your references and your goal. We pick the right pack together.'),('You book','Choose your slot. A deposit locks it in — the rest is paid on the day or in instalments.'),('We record','Warm-up, takes, doubles, ad-libs. You\'re coached until the take is the one.'),('We deliver','Mix and master within 7 days, one round of revisions included, ready for streaming.')],
  bk_e='Book', bk_h='Reserve your session', bk_p='Tell us which pack you want, your style and your preferred dates. We answer within 24 h with available slots.',
  hours=[('Tuesday – Saturday','13:00 – 22:00'),('Night sessions','On booking'),('Remote mix & master','Worldwide')],
  bk1='Request a session', bk2='Pick a beat first',
  fq_h='Good to know',
  faq=[('It\'s my first time in a studio — is that a problem?','Not at all. Sessions are coached: we warm up, work the flow and build the track step by step.'),('Can I bring my own instrumental?','Yes. Bring a WAV (and stems if you have them). Make sure you own the right license to release it.'),('Who owns the recordings?','You do. Your vocals and your masters are yours — we only ask for a production credit when we made the beat.'),('Can I pay in several times?','Yes, every pack can be paid in instalments. A deposit confirms the booking.')],
),
'fr': dict(
  title='HUMPIRE RECORDS — Studio d\'enregistrement, mix & master',
  desc='HUMPIRE RECORDS : studio d\'enregistrement de DJBILBOX BEATS. Enregistre, mixe et masterise ton single, ton EP ou ton album avec un beatmaker aux manettes. Packs à prix fixe dès 220 €.',
  eyebrow='Studio d\'enregistrement · by DJBILBOX BEATS', tag='Ta voix. Notre son.', tag2='Prêt à sortir.',
  lead='Enregistre, mixe et masterise ton morceau avec un producteur aux manettes — le même qui a fait plus de 800 instrus et les plugins HUMPIRE. Prix fixes, aucune surprise, un titre fini quand tu repars.',
  cta1='Voir les packs', cta2='Réserver une session',
  pil=[('Enregistrement','Cabine traitée, micro statique de studio, coaching sur chaque prise.'),('Mix & Master','Fort, propre, prêt pour la radio — un son qui tient sur Spotify et dans la voiture.'),('Instrus sur mesure','Une instru originale faite pour toi — G-Funk, Oriental, Afro, Trap, Drill.'),('Direction artistique','Topline, structure, flow, cover et un vrai plan de sortie.')],
  room_e='Le studio', room_h='Bienvenue chez HUMPIRE Records', room_s='Des LED, une cabine traitée et un salon pour se poser entre deux prises.',
  g=['La régie','La cabine','Le micro','Le salon'], galt=["Régie éclairée aux LED avec le logo HUMPIRE RECORDS sur l'écran mural",'Artiste qui enregistre dans la cabine LED','Micro dans la cabine LED','Salon du studio éclairé aux LED néon'],
  pr_e='Tarifs', pr_h='Les packs studio', pr_s='Prix fixes, tout compris. Paiement en une fois ou en plusieurs fois.',
  most='Le plus choisi', book='Réserver',
  packs=[('Single','Un titre, prêt à sortir',['2 h d\'enregistrement avec ingé son','Mix & master d\'1 titre','Export WAV + MP3','Conseils artistiques sur la prise']),
         ('Single + Instru','Ton instru à toi',['Instru sur mesure par DJBILBOX — exclusive','2 h d\'enregistrement avec ingé son','Mix & master d\'1 titre','WAV + MP3 + pistes de l\'instru']),
         ('EP','5 titres, un projet cohérent',['10 h d\'enregistrement avec ingé son','5 mixages + 5 masterings','Exports WAV + MP3','Tracklist & direction artistique']),
         ('Empire','Album / projet artiste complet',['10 titres, 20 h d\'enregistrement','10 mixages + 10 masterings','Cover offerte','Contenu backstage pour les réseaux','Plan de sortie & aide à la distribution'])],
  ca_e='À la carte', ca_h='Prestations à l\'unité',
  carte=[('Session d\'enregistrement avec ingé son','40 € / h'),('Bloc de 4 heures','150 €'),('Session de nuit (22 h – 6 h), par heure','50 € / h'),('Mixage d\'1 titre','90 €'),('Mastering d\'1 titre','35 €'),('Mix + master d\'1 titre (à distance, envoie tes pistes)','110 €'),('Instru exclusive sur mesure','200 €'),('Cover','60 €'),('Shooting photo · clip','Sur devis')],
  note='Le mix & master se fait aussi à distance, partout dans le monde — envoie tes pistes, récupère ton titre.',
  st_e='Comment ça marche', st_h='De l\'idée à la sortie',
  steps=[('On échange','Envoie ton projet, tes références et ton objectif. On choisit le bon pack ensemble.'),('Tu réserves','Choisis ton créneau. Un acompte le bloque — le reste se paie le jour même ou en plusieurs fois.'),('On enregistre','Échauffement, prises, doublages, ad-libs. On te coache jusqu\'à la bonne prise.'),('On livre','Mix et master sous 7 jours, une retouche incluse, prêt pour le streaming.')],
  bk_e='Réserver', bk_h='Réserve ta session', bk_p='Dis-nous quel pack tu veux, ton style et tes dates. On te répond sous 24 h avec les créneaux libres.',
  hours=[('Mardi – samedi','13 h – 22 h'),('Sessions de nuit','Sur réservation'),('Mix & master à distance','Partout dans le monde')],
  bk1='Demander une session', bk2='Choisir une instru',
  fq_h='Bon à savoir',
  faq=[('C\'est ma première fois en studio, c\'est un problème ?','Pas du tout. Les sessions sont coachées : on s\'échauffe, on travaille le flow et on construit le morceau étape par étape.'),('Je peux venir avec mon instru ?','Oui. Apporte un WAV (et les pistes si tu les as). Vérifie que tu as la bonne licence pour sortir le titre.'),('À qui appartiennent les enregistrements ?','À toi. Tes voix et tes masters sont à toi — on demande seulement un crédit de production quand l\'instru vient de nous.'),('Je peux payer en plusieurs fois ?','Oui, chaque pack peut se payer en plusieurs fois. Un acompte confirme la réservation.')],
),
'es': dict(
  title='HUMPIRE RECORDS — Estudio de grabación, mezcla y máster',
  desc='HUMPIRE RECORDS: estudio de grabación de DJBILBOX BEATS. Graba, mezcla y masteriza tu single, EP o álbum con un productor en la mesa. Packs a precio fijo desde 220 €.',
  eyebrow='Estudio de grabación · by DJBILBOX BEATS', tag='Tu voz. Nuestro sonido.', tag2='Listo para lanzar.',
  lead='Graba, mezcla y masteriza tu tema con un productor en la mesa — el mismo detrás de más de 800 beats y los plugins HUMPIRE. Precios fijos, sin sorpresas, un tema terminado cuando te vas.',
  cta1='Ver los packs', cta2='Reservar una sesión',
  pil=[('Grabación','Cabina tratada, micrófono de condensador de estudio, coaching en cada toma.'),('Mezcla y máster','Fuerte, limpio, listo para la radio — un sonido que aguanta en Spotify y en el coche.'),('Beats a medida','Un instrumental original hecho para ti — G-Funk, Oriental, Afro, Trap, Drill.'),('Dirección artística','Topline, estructura, flow, portada y un plan de lanzamiento.')],
  room_e='El estudio', room_h='Dentro de HUMPIRE Records', room_s='Luces LED, una cabina tratada y un salón para relajarte entre tomas.',
  g=['La sala de control','La cabina','El micro','El salón'], galt=['Sala de control con luces LED y el logo HUMPIRE RECORDS en la pantalla','Artista grabando en la cabina LED','Micrófono en la cabina LED','Salón del estudio con luces LED neón'],
  pr_e='Precios', pr_h='Packs de estudio', pr_s='Precios fijos, todo incluido. Paga de una vez o a plazos.',
  most='El más elegido', book='Reservar',
  packs=[('Single','Un tema, listo para lanzar',['2 h de grabación con ingeniero','Mezcla y máster de 1 tema','Exportación WAV + MP3','Consejos artísticos en la toma']),
         ('Single + Beat','Tu propio instrumental',['Beat a medida de DJBILBOX — exclusivo','2 h de grabación con ingeniero','Mezcla y máster de 1 tema','WAV + MP3 + stems del beat']),
         ('EP','5 temas, un proyecto coherente',['10 h de grabación con ingeniero','5 mezclas + 5 másters','Exportaciones WAV + MP3','Tracklist y dirección artística']),
         ('Empire','Álbum / proyecto completo',['10 temas, 20 h de grabación','10 mezclas + 10 másters','Portada incluida','Contenido backstage para redes','Plan de lanzamiento y ayuda con la distribución'])],
  ca_e='A la carta', ca_h='Servicios sueltos',
  carte=[('Sesión de grabación con ingeniero','40 € / h'),('Bloque de 4 horas','150 €'),('Sesión nocturna (22:00 – 06:00), por hora','50 € / h'),('Mezcla de 1 tema','90 €'),('Máster de 1 tema','35 €'),('Mezcla + máster de 1 tema (a distancia, envía tus pistas)','110 €'),('Beat exclusivo a medida','200 €'),('Portada','60 €'),('Sesión de fotos · videoclip','Bajo presupuesto')],
  note='La mezcla y el máster también se hacen a distancia, en todo el mundo — envía tus pistas y recibe tu tema.',
  st_e='Cómo funciona', st_h='De la idea al lanzamiento',
  steps=[('Hablamos','Envía tu proyecto, tus referencias y tu objetivo. Elegimos juntos el pack adecuado.'),('Reservas','Elige tu horario. Una señal lo bloquea — el resto se paga el día o a plazos.'),('Grabamos','Calentamiento, tomas, dobles, ad-libs. Te guiamos hasta la toma buena.'),('Entregamos','Mezcla y máster en 7 días, una ronda de cambios incluida, listo para streaming.')],
  bk_e='Reservar', bk_h='Reserva tu sesión', bk_p='Dinos qué pack quieres, tu estilo y tus fechas. Respondemos en 24 h con los horarios libres.',
  hours=[('Martes – sábado','13:00 – 22:00'),('Sesiones nocturnas','Con reserva'),('Mezcla y máster a distancia','Todo el mundo')],
  bk1='Pedir una sesión', bk2='Elegir un beat',
  fq_h='Bueno saber',
  faq=[('Es mi primera vez en un estudio, ¿es un problema?','Para nada. Las sesiones son guiadas: calentamos, trabajamos el flow y construimos el tema paso a paso.'),('¿Puedo traer mi propio instrumental?','Sí. Trae un WAV (y las pistas si las tienes). Asegúrate de tener la licencia correcta para lanzarlo.'),('¿De quién son las grabaciones?','Tuyas. Tus voces y tus másters son tuyos — solo pedimos el crédito de producción cuando el beat es nuestro.'),('¿Puedo pagar a plazos?','Sí, cada pack se puede pagar a plazos. Una señal confirma la reserva.')],
),
'pt': dict(
  title='HUMPIRE RECORDS — Estúdio de gravação, mix e master',
  desc='HUMPIRE RECORDS: estúdio de gravação de DJBILBOX BEATS. Grave, mixe e masterize seu single, EP ou álbum com um produtor na mesa. Pacotes a preço fixo a partir de 220 €.',
  eyebrow='Estúdio de gravação · by DJBILBOX BEATS', tag='Sua voz. Nosso som.', tag2='Pronto para lançar.',
  lead='Grave, mixe e masterize sua faixa com um produtor na mesa — o mesmo por trás de mais de 800 beats e dos plugins HUMPIRE. Preços fixos, sem surpresas, uma faixa pronta quando você sai.',
  cta1='Ver os pacotes', cta2='Reservar uma sessão',
  pil=[('Gravação','Cabine tratada, microfone condensador de estúdio, coaching em cada take.'),('Mix e master','Alto, limpo, pronto para a rádio — um som que funciona no Spotify e no carro.'),('Beats sob medida','Um instrumental original feito para você — G-Funk, Oriental, Afro, Trap, Drill.'),('Direção artística','Topline, estrutura, flow, capa e um plano de lançamento.')],
  room_e='O estúdio', room_h='Dentro da HUMPIRE Records', room_s='Luzes LED, uma cabine tratada e um lounge para relaxar entre os takes.',
  g=['A técnica','A cabine','O microfone','O lounge'], galt=['Sala técnica com luzes LED e o logo HUMPIRE RECORDS na tela da parede','Artista gravando na cabine LED','Microfone na cabine LED','Lounge do estúdio com luzes LED neon'],
  pr_e='Preços', pr_h='Pacotes de estúdio', pr_s='Preços fixos, tudo incluído. Pague de uma vez ou parcelado.',
  most='O mais escolhido', book='Reservar',
  packs=[('Single','Uma faixa, pronta para lançar',['2 h de gravação com engenheiro','Mix e master de 1 faixa','Exportação WAV + MP3','Dicas artísticas no take']),
         ('Single + Beat','Seu próprio instrumental',['Beat sob medida por DJBILBOX — exclusivo','2 h de gravação com engenheiro','Mix e master de 1 faixa','WAV + MP3 + stems do beat']),
         ('EP','5 faixas, um projeto coerente',['10 h de gravação com engenheiro','5 mixes + 5 masters','Exportações WAV + MP3','Tracklist e direção artística']),
         ('Empire','Álbum / projeto completo',['10 faixas, 20 h de gravação','10 mixes + 10 masters','Capa incluída','Conteúdo de bastidores para as redes','Plano de lançamento e apoio na distribuição'])],
  ca_e='À la carte', ca_h='Serviços avulsos',
  carte=[('Sessão de gravação com engenheiro','40 € / h'),('Bloco de 4 horas','150 €'),('Sessão noturna (22:00 – 06:00), por hora','50 € / h'),('Mix de 1 faixa','90 €'),('Master de 1 faixa','35 €'),('Mix + master de 1 faixa (à distância, envie suas trilhas)','110 €'),('Beat exclusivo sob medida','200 €'),('Capa','60 €'),('Ensaio fotográfico · videoclipe','Sob orçamento')],
  note='Mix e master também à distância, no mundo inteiro — envie suas trilhas e receba sua faixa.',
  st_e='Como funciona', st_h='Da ideia ao lançamento',
  steps=[('Conversamos','Envie seu projeto, suas referências e seu objetivo. Escolhemos juntos o pacote certo.'),('Você reserva','Escolha seu horário. Um sinal garante a vaga — o resto é pago no dia ou parcelado.'),('Gravamos','Aquecimento, takes, dobras, ad-libs. Você é orientado até o take certo.'),('Entregamos','Mix e master em 7 dias, uma rodada de ajustes incluída, pronto para streaming.')],
  bk_e='Reservar', bk_h='Reserve sua sessão', bk_p='Diga qual pacote você quer, seu estilo e suas datas. Respondemos em 24 h com os horários livres.',
  hours=[('Terça – sábado','13:00 – 22:00'),('Sessões noturnas','Com reserva'),('Mix e master à distância','Mundo inteiro')],
  bk1='Pedir uma sessão', bk2='Escolher um beat',
  fq_h='Bom saber',
  faq=[('É minha primeira vez em estúdio, tem problema?','Nenhum. As sessões são orientadas: aquecemos, trabalhamos o flow e construímos a faixa passo a passo.'),('Posso levar meu próprio instrumental?','Sim. Traga um WAV (e as trilhas se tiver). Confirme que você tem a licença certa para lançar.'),('De quem são as gravações?','Suas. Suas vozes e seus masters são seus — só pedimos crédito de produção quando o beat é nosso.'),('Posso pagar parcelado?','Sim, todo pacote pode ser parcelado. Um sinal confirma a reserva.')],
),
'de': dict(
  title='HUMPIRE RECORDS — Tonstudio, Mix & Master',
  desc='HUMPIRE RECORDS: Tonstudio von DJBILBOX BEATS. Nimm deine Single, deine EP oder dein Album mit einem Produzenten am Pult auf, inklusive Mix und Master. Festpreis-Pakete ab 220 €.',
  eyebrow='Tonstudio · by DJBILBOX BEATS', tag='Deine Stimme. Unser Sound.', tag2='Bereit für den Release.',
  lead='Nimm deinen Track auf, lass ihn mischen und mastern — mit einem Produzenten am Pult, derselbe hinter 800+ Beats und den HUMPIRE-Plugins. Festpreise, keine Überraschungen, ein fertiger Song, wenn du gehst.',
  cta1='Pakete ansehen', cta2='Session buchen',
  pil=[('Aufnahme','Akustisch optimierte Gesangskabine, Studio-Kondensatormikrofon, Coaching bei jedem Take.'),('Mix & Master','Laut, sauber, radiotauglich — ein Sound, der auf Spotify und im Auto funktioniert.'),('Beats nach Maß','Ein originales Instrumental für dich — G-Funk, Oriental, Afro, Trap, Drill.'),('Künstlerische Leitung','Topline, Struktur, Flow, Cover und ein Plan für den Release.')],
  room_e='Das Studio', room_h='Willkommen bei HUMPIRE Records', room_s='LED-Licht, eine optimierte Kabine und eine Lounge zum Chillen zwischen den Takes.',
  g=['Die Regie','Die Kabine','Das Mikro','Die Lounge'], galt=['Regieraum mit LED-Licht und dem HUMPIRE RECORDS Logo auf dem Wandbildschirm','Künstler bei der Aufnahme in der LED-Kabine','Mikrofon in der LED-Gesangskabine','Studio-Lounge mit Neon-LED-Licht'],
  pr_e='Preise', pr_h='Studio-Pakete', pr_s='Festpreise, alles inklusive. Auf einmal oder in Raten zahlen.',
  most='Am beliebtesten', book='Buchen',
  packs=[('Single','Ein Track, bereit für den Release',['2 Std. Aufnahme mit Toningenieur','Mix & Master von 1 Track','Export WAV + MP3','Künstlerisches Feedback beim Take']),
         ('Single + Beat','Dein eigenes Instrumental',['Beat nach Maß von DJBILBOX — exklusiv','2 Std. Aufnahme mit Toningenieur','Mix & Master von 1 Track','WAV + MP3 + Beat-Stems']),
         ('EP','5 Tracks, ein stimmiges Projekt',['10 Std. Aufnahme mit Toningenieur','5 Mixe + 5 Master','Exporte WAV + MP3','Tracklist & künstlerische Leitung']),
         ('Empire','Album / komplettes Projekt',['10 Tracks, 20 Std. Aufnahme','10 Mixe + 10 Master','Cover inklusive','Backstage-Content für Social Media','Release-Plan & Hilfe beim Vertrieb'])],
  ca_e='À la carte', ca_h='Einzelleistungen',
  carte=[('Aufnahme-Session mit Toningenieur','40 € / Std.'),('4-Stunden-Block','150 €'),('Nacht-Session (22:00 – 06:00), pro Stunde','50 € / Std.'),('Mix von 1 Track','90 €'),('Master von 1 Track','35 €'),('Mix + Master von 1 Track (online, schick deine Spuren)','110 €'),('Exklusiver Beat nach Maß','200 €'),('Cover','60 €'),('Fotoshooting · Musikvideo','Auf Anfrage')],
  note='Mix & Master gibt es auch online, weltweit — schick deine Spuren und bekomm deinen Track zurück.',
  st_e='So läuft es', st_h='Von der Idee zum Release',
  steps=[('Wir reden','Schick dein Projekt, deine Referenzen und dein Ziel. Wir wählen gemeinsam das richtige Paket.'),('Du buchst','Wähl deinen Termin. Eine Anzahlung sichert ihn — der Rest wird am Tag oder in Raten bezahlt.'),('Wir nehmen auf','Aufwärmen, Takes, Doubles, Ad-libs. Du wirst gecoacht, bis der Take sitzt.'),('Wir liefern','Mix und Master in 7 Tagen, eine Korrekturrunde inklusive, bereit fürs Streaming.')],
  bk_e='Buchen', bk_h='Reserviere deine Session', bk_p='Sag uns, welches Paket du willst, deinen Stil und deine Wunschtermine. Wir antworten innerhalb von 24 Std. mit freien Terminen.',
  hours=[('Dienstag – Samstag','13:00 – 22:00'),('Nacht-Sessions','Auf Buchung'),('Online Mix & Master','Weltweit')],
  bk1='Session anfragen', bk2='Erst einen Beat wählen',
  fq_h='Gut zu wissen',
  faq=[('Ich war noch nie im Studio — ist das ein Problem?','Überhaupt nicht. Die Sessions sind gecoacht: Wir wärmen uns auf, arbeiten am Flow und bauen den Track Schritt für Schritt.'),('Kann ich mein eigenes Instrumental mitbringen?','Ja. Bring eine WAV (und Stems, falls vorhanden). Achte darauf, dass du die richtige Lizenz für den Release hast.'),('Wem gehören die Aufnahmen?','Dir. Deine Vocals und deine Master gehören dir — wir bitten nur um einen Producer-Credit, wenn der Beat von uns ist.'),('Kann ich in Raten zahlen?','Ja, jedes Paket kann in Raten bezahlt werden. Eine Anzahlung bestätigt die Buchung.')],
),
}

T.update({
'ar': dict(
  title='HUMPIRE RECORDS — استوديو تسجيل، ميكس وماستر',
  desc='HUMPIRE RECORDS: استوديو تسجيل من DJBILBOX BEATS. سجّل ودمج وأتقن أغنيتك أو EP أو ألبومك مع منتج خلف الطاولة. باقات بسعر ثابت تبدأ من 220 يورو.',
  eyebrow='استوديو تسجيل · by DJBILBOX BEATS', tag='صوتك. صوتنا.', tag2='جاهز للإصدار.',
  lead='سجّل أغنيتك واعمل لها ميكس وماستر مع منتج خلف الطاولة — نفس اليد وراء أكثر من 800 بيت وإضافات HUMPIRE. أسعار ثابتة، بدون مفاجآت، وأغنية جاهزة عند خروجك.',
  cta1='شاهد الباقات', cta2='احجز جلسة',
  pil=[('التسجيل','كابينة صوت معالجة، ميكروفون مكثّف احترافي، وتوجيه في كل تسجيلة.'),('ميكس وماستر','صوت قوي ونظيف وجاهز للراديو — يبقى رائعاً على Spotify وفي السيارة.'),('بيتات حسب الطلب','موسيقى أصلية مصنوعة لك — G-Funk، شرقي، أفرو، تراب، دريل.'),('إدارة فنية','الألحان، البنية، الفلو، الغلاف، وخطة إصدار حقيقية.')],
  room_e='الاستوديو', room_h='داخل HUMPIRE Records', room_s='إضاءة LED، كابينة معالجة، وصالون للاسترخاء بين التسجيلات.',
  g=['غرفة التحكم','الكابينة','الميكروفون','الصالون'], galt=['غرفة التحكم بإضاءة LED وشعار HUMPIRE RECORDS على الشاشة','فنان يسجّل في كابينة LED','ميكروفون في كابينة LED','صالون الاستوديو بإضاءة نيون'],
  pr_e='الأسعار', pr_h='باقات الاستوديو', pr_s='أسعار ثابتة، كل شيء مشمول. ادفع مرة واحدة أو على دفعات.',
  most='الأكثر طلباً', book='احجز',
  packs=[('Single','أغنية واحدة جاهزة للإصدار',['ساعتان تسجيل مع مهندس صوت','ميكس وماستر لأغنية واحدة','تصدير WAV + MP3','نصائح فنية أثناء التسجيل']),
         ('Single + Beat','بيت خاص بك',['بيت حسب الطلب من DJBILBOX — حصري','ساعتان تسجيل مع مهندس صوت','ميكس وماستر لأغنية واحدة','WAV + MP3 + مسارات البيت']),
         ('EP','5 أغانٍ، مشروع متكامل',['10 ساعات تسجيل مع مهندس صوت','5 ميكس + 5 ماستر','تصدير WAV + MP3','ترتيب الأغاني وإدارة فنية']),
         ('Empire','ألبوم / مشروع فني كامل',['10 أغانٍ، 20 ساعة تسجيل','10 ميكس + 10 ماستر','غلاف مشمول','محتوى كواليس للسوشيال ميديا','خطة إصدار ومساعدة في التوزيع'])],
  ca_e='خدمات منفصلة', ca_h='الخدمات بالقطعة',
  carte=[('جلسة تسجيل مع مهندس صوت','40 € / ساعة'),('باقة 4 ساعات','150 €'),('جلسة ليلية (22:00 – 06:00)، للساعة','50 € / ساعة'),('ميكس لأغنية واحدة','90 €'),('ماستر لأغنية واحدة','35 €'),('ميكس + ماستر لأغنية (عن بُعد، أرسل مساراتك)','110 €'),('بيت حصري حسب الطلب','200 €'),('غلاف','60 €'),('جلسة تصوير · فيديو كليب','حسب الطلب')],
  note='الميكس والماستر متاحان أيضاً عن بُعد، من أي مكان في العالم — أرسل مساراتك واستلم أغنيتك.',
  st_e='كيف نعمل', st_h='من الفكرة إلى الإصدار',
  steps=[('نتحدث','أرسل مشروعك ومراجعك وهدفك. نختار معاً الباقة المناسبة.'),('تحجز','اختر موعدك. عربون يثبّت الحجز — والباقي يُدفع يوم الجلسة أو على دفعات.'),('نسجّل','تسخين، تسجيلات، طبقات، أد-ليبس. نوجّهك حتى نحصل على أفضل تسجيلة.'),('نسلّم','ميكس وماستر خلال 7 أيام، مع تعديل واحد مشمول، جاهز للبث.')],
  bk_e='احجز', bk_h='احجز جلستك', bk_p='أخبرنا بالباقة التي تريدها وأسلوبك والتواريخ المناسبة. نرد خلال 24 ساعة بالمواعيد المتاحة.',
  hours=[('الثلاثاء – السبت','13:00 – 22:00'),('جلسات ليلية','بالحجز'),('ميكس وماستر عن بُعد','في كل العالم')],
  bk1='اطلب جلسة', bk2='اختر بيت أولاً',
  fq_h='معلومات مفيدة',
  faq=[('هذه أول مرة لي في استوديو — هل هذه مشكلة؟','أبداً. الجلسات موجّهة: نبدأ بالتسخين، نعمل على الفلو ونبني الأغنية خطوة بخطوة.'),('هل يمكنني إحضار البيت الخاص بي؟','نعم. أحضر ملف WAV (والمسارات إن وجدت). تأكد أن لديك الرخصة المناسبة للإصدار.'),('لمن تعود التسجيلات؟','لك. أصواتك والماستر ملكك — نطلب فقط ذكر اسم المنتج عندما يكون البيت من صنعنا.'),('هل يمكنني الدفع على دفعات؟','نعم، كل باقة يمكن دفعها على دفعات. العربون يؤكد الحجز.')],
),
'ru': dict(
  title='HUMPIRE RECORDS — студия звукозаписи, сведение и мастеринг',
  desc='HUMPIRE RECORDS — студия звукозаписи DJBILBOX BEATS. Запиши, сведи и отмастери сингл, EP или альбом с продюсером за пультом. Пакеты по фиксированной цене от 220 €.',
  eyebrow='Студия звукозаписи · by DJBILBOX BEATS', tag='Твой голос. Наш звук.', tag2='Готово к релизу.',
  lead='Запиши, сведи и отмастери трек с продюсером за пультом — тем самым, кто сделал 800+ битов и плагины HUMPIRE. Фиксированные цены, никаких сюрпризов, готовый трек, когда уходишь.',
  cta1='Смотреть пакеты', cta2='Забронировать сессию',
  pil=[('Запись','Акустически обработанная вокальная кабина, студийный конденсаторный микрофон, помощь на каждом дубле.'),('Сведение и мастеринг','Громко, чисто, готово для радио — звук, который держится и в Spotify, и в машине.'),('Биты на заказ','Оригинальный инструментал для тебя — G-Funk, восточный, афро, трэп, дрилл.'),('Арт-дирекция','Топлайн, структура, флоу, обложка и план релиза.')],
  room_e='Студия', room_h='Внутри HUMPIRE Records', room_s='LED-подсветка, обработанная кабина и лаунж, чтобы отдохнуть между дублями.',
  g=['Аппаратная','Кабина','Микрофон','Лаунж'], galt=['Аппаратная с LED-подсветкой и логотипом HUMPIRE RECORDS на экране','Артист записывается в LED-кабине','Микрофон в LED-кабине','Лаунж студии с неоновой подсветкой'],
  pr_e='Цены', pr_h='Студийные пакеты', pr_s='Фиксированные цены, всё включено. Оплата сразу или частями.',
  most='Самый популярный', book='Забронировать',
  packs=[('Single','Один трек, готовый к релизу',['2 ч записи со звукорежиссёром','Сведение и мастеринг 1 трека','Экспорт WAV + MP3','Советы по исполнению']),
         ('Single + Beat','Твой собственный бит',['Бит на заказ от DJBILBOX — эксклюзив','2 ч записи со звукорежиссёром','Сведение и мастеринг 1 трека','WAV + MP3 + стемы бита']),
         ('EP','5 треков, цельный проект',['10 ч записи со звукорежиссёром','5 сведений + 5 мастерингов','Экспорт WAV + MP3','Треклист и арт-дирекция']),
         ('Empire','Альбом / полный проект артиста',['10 треков, 20 ч записи','10 сведений + 10 мастерингов','Обложка в подарок','Бэкстейдж-контент для соцсетей','План релиза и помощь с дистрибуцией'])],
  ca_e='À la carte', ca_h='Отдельные услуги',
  carte=[('Сессия записи со звукорежиссёром','40 € / ч'),('Блок 4 часа','150 €'),('Ночная сессия (22:00 – 06:00), за час','50 € / ч'),('Сведение 1 трека','90 €'),('Мастеринг 1 трека','35 €'),('Сведение + мастеринг 1 трека (удалённо, пришли дорожки)','110 €'),('Эксклюзивный бит на заказ','200 €'),('Обложка','60 €'),('Фотосессия · клип','По запросу')],
  note='Сведение и мастеринг доступны и удалённо, по всему миру — пришли дорожки и получи готовый трек.',
  st_e='Как это работает', st_h='От идеи до релиза',
  steps=[('Обсуждаем','Пришли проект, референсы и цель. Вместе выберем подходящий пакет.'),('Бронируешь','Выбери время. Предоплата закрепляет его — остальное в день сессии или частями.'),('Записываем','Разогрев, дубли, даблы, эдлибы. Помогаем, пока дубль не станет тем самым.'),('Сдаём','Сведение и мастеринг за 7 дней, одна правка включена, готово для стриминга.')],
  bk_e='Бронь', bk_h='Забронируй сессию', bk_p='Напиши, какой пакет хочешь, свой стиль и удобные даты. Ответим за 24 часа со свободным временем.',
  hours=[('Вторник – суббота','13:00 – 22:00'),('Ночные сессии','По брони'),('Удалённое сведение и мастеринг','По всему миру')],
  bk1='Запросить сессию', bk2='Сначала выбрать бит',
  fq_h='Полезно знать',
  faq=[('Я впервые в студии — это проблема?','Совсем нет. Сессии с сопровождением: разогреваемся, работаем над флоу и строим трек шаг за шагом.'),('Можно прийти со своим битом?','Да. Принеси WAV (и стемы, если есть). Убедись, что у тебя есть лицензия на релиз.'),('Кому принадлежат записи?','Тебе. Вокал и мастеры — твои; мы просим только указать продюсера, если бит наш.'),('Можно платить частями?','Да, любой пакет можно оплатить частями. Предоплата подтверждает бронь.')],
),
'ja': dict(
  title='HUMPIRE RECORDS — レコーディングスタジオ、ミックス＆マスタリング',
  desc='HUMPIRE RECORDS：DJBILBOX BEATS のレコーディングスタジオ。プロデューサーと一緒にシングル、EP、アルバムを録音・ミックス・マスタリング。定額パックは220ユーロから。',
  eyebrow='レコーディングスタジオ · by DJBILBOX BEATS', tag='君の声。僕らのサウンド。', tag2='リリースの準備はできている。',
  lead='プロデューサーと一緒に、録音からミックス、マスタリングまで — 800以上のビートと HUMPIRE プラグインを作った本人が担当。定額制で追加料金なし、帰る頃には曲が完成。',
  cta1='パックを見る', cta2='セッションを予約',
  pil=[('レコーディング','音響処理されたボーカルブース、スタジオ用コンデンサーマイク、テイクごとのコーチング。'),('ミックス＆マスタリング','大きく、クリアで、ラジオ品質 — Spotify でも車でも映えるサウンド。'),('オーダーメイドのビート','君のためのオリジナル曲 — G-Funk、オリエンタル、アフロ、トラップ、ドリル。'),('アートディレクション','トップライン、構成、フロウ、ジャケット、そしてリリース計画。')],
  room_e='スタジオ', room_h='HUMPIRE Records の中へ', room_s='LEDライト、音響処理されたブース、テイクの合間にくつろげるラウンジ。',
  g=['コントロールルーム','ブース','マイク','ラウンジ'], galt=['LEDライトと画面に HUMPIRE RECORDS ロゴがあるコントロールルーム','LEDブースで録音するアーティスト','LEDブースのマイク','ネオンLEDのスタジオラウンジ'],
  pr_e='料金', pr_h='スタジオパック', pr_s='定額・すべて込み。一括または分割払い。',
  most='一番人気', book='予約する',
  packs=[('Single','リリースできる1曲',['エンジニア付き録音2時間','1曲のミックス＆マスタリング','WAV + MP3 書き出し','テイクへのアドバイス']),
         ('Single + Beat','君だけのビート',['DJBILBOX のオーダーメイドビート（独占）','エンジニア付き録音2時間','1曲のミックス＆マスタリング','WAV + MP3 + ビートのステム']),
         ('EP','5曲、まとまった作品',['エンジニア付き録音10時間','ミックス5曲 + マスタリング5曲','WAV + MP3 書き出し','曲順とアートディレクション']),
         ('Empire','アルバム／アーティストの全プロジェクト',['10曲、録音20時間','ミックス10曲 + マスタリング10曲','ジャケット込み','SNS用のバックステージ素材','リリース計画と配信サポート'])],
  ca_e='単品', ca_h='単品サービス',
  carte=[('エンジニア付きレコーディング','40 € / 時間'),('4時間パック','150 €'),('深夜セッション（22:00〜06:00）1時間','50 € / 時間'),('1曲のミックス','90 €'),('1曲のマスタリング','35 €'),('1曲のミックス＋マスタリング（リモート、パラデータ送付）','110 €'),('独占オーダーメイドビート','200 €'),('ジャケット','60 €'),('写真撮影・MV','お見積り')],
  note='ミックスとマスタリングは世界中からリモートでも対応 — パラデータを送れば完成曲が届きます。',
  st_e='流れ', st_h='アイデアからリリースまで',
  steps=[('相談','プロジェクト、参考曲、目標を送ってください。一緒に最適なパックを選びます。'),('予約','時間を選んでください。前金で確定 — 残りは当日または分割で。'),('録音','ウォームアップ、テイク、ダブル、アドリブ。最高のテイクまでサポートします。'),('納品','7日以内にミックス＆マスタリング、修正1回込み、配信の準備完了。')],
  bk_e='予約', bk_h='セッションを予約', bk_p='希望のパック、スタイル、日程を教えてください。24時間以内に空き枠をご連絡します。',
  hours=[('火曜〜土曜','13:00〜22:00'),('深夜セッション','予約制'),('リモートのミックス＆マスタリング','世界中から')],
  bk1='セッションを依頼', bk2='先にビートを選ぶ',
  fq_h='よくある質問',
  faq=[('スタジオは初めてですが大丈夫？','もちろん。セッションはコーチ付き：ウォームアップしてフロウを練り、一歩ずつ曲を作ります。'),('自分のビートを持ち込めますか？','はい。WAV（あればステムも）を持ってきてください。リリースに必要なライセンスがあるか確認を。'),('録音の権利は誰のもの？','あなたです。ボーカルとマスター音源はあなたのもの — ビートが私たちのものの場合のみプロデューサークレジットをお願いします。'),('分割払いはできますか？','はい、すべてのパックは分割払い可能です。前金で予約が確定します。')],
),
})
# Real home studio (24/11/2026): only the user's own gear is shown.
REAL = {
 'en': ('Inside HUMPIRE Records', 'A real home studio: black acoustic foam on the walls, a tall mic stand and LED lights for the vibe.',
        ['The desk', 'The vocal corner'],
        ['Home studio desk with gaming PC, Novation FLkey keyboard, audio interface and a TV on the wall', 'Condenser microphone on a tall stand in front of black acoustic foam'],
        'Vocal corner treated with acoustic foam, condenser mic on a tall stand, coaching on every take.'),
 'fr': ('Bienvenue chez HUMPIRE Records', 'Un vrai home studio : mousse acoustique noire aux murs, grand pied de micro et LED pour l\'ambiance.',
        ['Le bureau', 'Le coin voix'],
        ['Bureau du home studio avec PC gamer, clavier Novation FLkey, carte son et TV au mur', 'Micro statique sur grand pied devant la mousse acoustique noire'],
        'Coin voix traité en mousse acoustique, micro statique sur grand pied, coaching sur chaque prise.'),
 'es': ('Dentro de HUMPIRE Records', 'Un home studio real: espuma acústica negra en las paredes, un pie de micro alto y luces LED para el ambiente.',
        ['El escritorio', 'El rincón de voz'],
        ['Escritorio del home studio con PC gamer, teclado Novation FLkey, interfaz de audio y TV en la pared', 'Micrófono de condensador en un pie alto delante de la espuma acústica negra'],
        'Rincón de voz tratado con espuma acústica, micrófono de condensador en pie alto, coaching en cada toma.'),
 'pt': ('Dentro da HUMPIRE Records', 'Um home studio de verdade: espuma acústica preta nas paredes, pedestal de microfone alto e luzes LED para a vibe.',
        ['A mesa', 'O canto de voz'],
        ['Mesa do home studio com PC gamer, teclado Novation FLkey, interface de áudio e TV na parede', 'Microfone condensador em pedestal alto na frente da espuma acústica preta'],
        'Canto de voz tratado com espuma acústica, microfone condensador em pedestal alto, coaching em cada take.'),
 'de': ('Willkommen bei HUMPIRE Records', 'Ein echtes Homestudio: schwarzer Akustikschaum an den Wänden, ein hohes Mikrofonstativ und LED-Licht für die Stimmung.',
        ['Der Arbeitsplatz', 'Die Gesangsecke'],
        ['Homestudio-Schreibtisch mit Gaming-PC, Novation-FLkey-Keyboard, Audio-Interface und Wand-TV', 'Kondensatormikrofon auf hohem Stativ vor schwarzem Akustikschaum'],
        'Gesangsecke mit Akustikschaum, Kondensatormikrofon auf hohem Stativ, Coaching bei jedem Take.'),
 'ar': ('داخل HUMPIRE Records', 'استوديو منزلي حقيقي: إسفنج عازل أسود على الجدران، حامل ميكروفون طويل وإضاءة LED للأجواء.',
        ['المكتب', 'ركن الصوت'],
        ['مكتب الاستوديو المنزلي مع كمبيوتر ألعاب ولوحة Novation FLkey وكرت صوت وتلفاز على الجدار', 'ميكروفون مكثّف على حامل طويل أمام إسفنج عازل أسود'],
        'ركن صوت معالج بالإسفنج العازل، ميكروفون مكثّف على حامل طويل، وتوجيه في كل تسجيلة.'),
 'ru': ('Внутри HUMPIRE Records', 'Настоящая домашняя студия: чёрный акустический поролон на стенах, высокая микрофонная стойка и LED-подсветка.',
        ['Рабочее место', 'Вокальный угол'],
        ['Стол домашней студии: игровой ПК, клавиатура Novation FLkey, аудиоинтерфейс и телевизор на стене', 'Конденсаторный микрофон на высокой стойке перед чёрным акустическим поролоном'],
        'Вокальный угол с акустическим поролоном, конденсаторный микрофон на высокой стойке, помощь на каждом дубле.'),
 'ja': ('HUMPIRE Records の中へ', '本物のホームスタジオ：壁には黒い吸音材、背の高いマイクスタンド、雰囲気を作るLEDライト。',
        ['デスク', 'ボーカルコーナー'],
        ['ゲーミングPC、Novation FLkey、オーディオインターフェース、壁掛けTVのあるホームスタジオのデスク', '黒い吸音材の前、背の高いスタンドに付けたコンデンサーマイク'],
        '吸音材で処理したボーカルコーナー、背の高いスタンドのコンデンサーマイク、テイクごとのコーチング。'),
}
for _l, (_h, _s, _g, _alt, _p0) in REAL.items():
    T[_l]['room_h'], T[_l]['room_s'], T[_l]['g'], T[_l]['galt'] = _h, _s, _g, _alt
    T[_l]['pil'][0] = (T[_l]['pil'][0][0], _p0)
LANGS += ['ar', 'ru', 'ja']
LABEL.update({'pt': 'PT-BR', 'ar': 'AR', 'ru': 'RU', 'ja': 'JA'})
RTL = {'ar'}
HREF = {'pt': 'pt-BR'}

def e(s): return html.escape(s, quote=False)

def build(lang):
    t = T[lang]; pre = '' if lang == 'en' else '../'
    alts = '\n'.join(f'<link rel="alternate" hreflang="{l}" href="{BASE}/{path(l)}">' for l in LANGS)
    alts += f'\n<link rel="alternate" hreflang="x-default" href="{BASE}/studio.html">'
    langs = ''.join(f'<a href="/{path(l)}?lang={l}"{" class=\"on\"" if l==lang else ""} hreflang="{l}">{LABEL[l]}</a>' for l in LANGS)
    pil = ''.join(f'<div><i class="fa-solid {ic}"></i><h3>{e(a)}</h3><p>{e(b)}</p></div>'
                  for ic,(a,b) in zip(['fa-microphone-lines','fa-sliders','fa-compact-disc','fa-crown'], t['pil']))
    imgs = ['home-desk.jpg','home-mic.jpg']
    gal = ''.join(f'<figure{" class=\"big\"" if i==0 else ""}><img src="{pre}img/studio/{imgs[i]}" alt="{e(t["galt"][i])}" loading="lazy"><figcaption>{e(t["g"][i])}</figcaption></figure>' for i in range(len(imgs)))
    packs = ''
    for i,(name,sub,items) in enumerate(t['packs']):
        price = [P['single'],P['beat'],P['ep'],P['empire']][i]
        star = i == 2
        lis = ''.join(f'<li>{e(x)}</li>' for x in items)
        packs += (f'<article class="hr-pack{" star" if star else ""}">' + (f'<span class="hr-ribbon">{e(t["most"])}</span>' if star else '') +
                  f'<h3>{e(name)}</h3><p class="hr-pack-for">{e(sub)}</p><div class="hr-price"><b>{price}</b><span>€</span></div>'
                  f'<ul>{lis}</ul><a class="hr-btn{"" if star else " ghost"}" href="#book">{e(t["book"])}</a></article>\n')
    carte = ''.join(f'<div class="hr-row"><span>{e(a)}</span><i></i><b>{e(b)}</b></div>' for a,b in t['carte'])
    steps = ''.join(f'<li><b>0{i+1}</b><h3>{e(a)}</h3><p>{e(b)}</p></li>' for i,(a,b) in enumerate(t['steps']))
    hours = ''.join(f'<li><span>{e(a)}</span><b>{e(b)}</b></li>' for a,b in t['hours'])
    faq = ''.join(f'<details><summary>{e(q)}</summary><p>{e(a)}</p></details>' for q,a in t['faq'])
    return f'''<!DOCTYPE html>
<html lang="{HREF.get(lang,lang)}"{' dir="rtl"' if lang in RTL else ''}>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>{e(t["title"])}</title>
<meta name="description" content="{html.escape(t["desc"])}">
<link rel="canonical" href="{BASE}/{path(lang)}">
{alts}
<meta property="og:title" content="{html.escape(t["title"])}">
<meta property="og:description" content="{html.escape(t["desc"])}">
<meta property="og:image" content="{BASE}/img/studio/home-desk.jpg">
<link rel="icon" href="/favicon.png">
<link rel="preconnect" href="https://fonts.googleapis.com"><link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Unbounded:wght@600;700;800&family=Inter:wght@400;500;600;700&display=swap">
<link rel="stylesheet" href="{pre}assets/css/theme.css?v=26110401">
<link rel="stylesheet" href="{pre}assets/css/theme-neon.css?v=26110401">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css">
<link rel="stylesheet" href="{pre}assets/css/studio.css?v=26112403">
</head>
<body data-page="studio" class="studio-page">
<!-- generated by tools/build_studio.py — edit the script, not this file -->
<main class="hr">
  <nav class="hr-langs" aria-label="Language">{langs}</nav>

  <section class="hr-hero">
    <div class="hr-hero-bg" aria-hidden="true"></div>
    <div class="hr-hero-glow" aria-hidden="true"></div>
    <div class="hr-meter" aria-hidden="true">{"<i></i>"*16}</div>
    <p class="hr-eyebrow">{e(t["eyebrow"])}</p>
    <h1 class="hr-logo">HUMPIRE<em>Records</em></h1>
    <p class="hr-tag">{e(t["tag"])} <span>{e(t["tag2"])}</span></p>
    <p class="hr-lead">{e(t["lead"])}</p>
    <div class="hr-cta">
      <a class="hr-btn" href="#packs">{e(t["cta1"])} <i class="fa-solid fa-arrow-down"></i></a>
      <a class="hr-btn ghost" href="#book"><i class="fa-regular fa-calendar"></i> {e(t["cta2"])}</a>
    </div>
  </section>

  <section class="hr-pillars">{pil}</section>

  <section class="hr-sec" id="room">
    <p class="hr-eyebrow">{e(t["room_e"])}</p>
    <h2>{e(t["room_h"])}</h2>
    <p class="hr-sub">{e(t["room_s"])}</p>
    <div class="hr-gallery">{gal}</div>
  </section>

  <section class="hr-sec" id="packs">
    <p class="hr-eyebrow">{e(t["pr_e"])}</p>
    <h2>{e(t["pr_h"])}</h2>
    <p class="hr-sub">{e(t["pr_s"])}</p>
    <div class="hr-packs">
{packs}    </div>
  </section>

  <section class="hr-sec" id="carte">
    <p class="hr-eyebrow">{e(t["ca_e"])}</p>
    <h2>{e(t["ca_h"])}</h2>
    <div class="hr-carte">{carte}</div>
    <p class="hr-note">{e(t["note"])}</p>
  </section>

  <section class="hr-sec">
    <p class="hr-eyebrow">{e(t["st_e"])}</p>
    <h2>{e(t["st_h"])}</h2>
    <ol class="hr-steps">{steps}</ol>
  </section>

  <section class="hr-book" id="book">
    <div>
      <p class="hr-eyebrow">{e(t["bk_e"])}</p>
      <h2>{e(t["bk_h"])}</h2>
      <p>{e(t["bk_p"])}</p>
      <ul class="hr-hours">{hours}</ul>
    </div>
    <div class="hr-book-cta">
      <a class="hr-btn" href="/contact.html"><i class="fa-regular fa-envelope"></i> {e(t["bk1"])}</a>
      <a class="hr-btn ghost" href="/beats.html"><i class="fa-solid fa-music"></i> {e(t["bk2"])}</a>
    </div>
  </section>

  <section class="hr-sec hr-faq">
    <p class="hr-eyebrow">FAQ</p>
    <h2>{e(t["fq_h"])}</h2>
    {faq}
  </section>
</main>

<script src="{pre}assets/js/site.js?v=26112402"></script>
</body>
</html>
'''

if __name__ == '__main__':
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    for l in LANGS:
        out = os.path.join(root, path(l))
        os.makedirs(os.path.dirname(out), exist_ok=True)
        open(out, 'w', encoding='utf-8', newline='\n').write(build(l))
        print('wrote', path(l))
