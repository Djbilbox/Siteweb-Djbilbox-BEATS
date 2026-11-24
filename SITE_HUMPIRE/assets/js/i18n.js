/* ============================================================
   HUMPIRE i18n — EN · FR · ES · PT · DE
   Hand-written dictionary applied to the page text (and to the
   content scripts add later). Choice: ?lang=xx > saved > browser.
   Pages that already exist per language (studio) are redirected.
   Add a line to D to translate a new string: "English": [fr, es, pt, de]
   ============================================================ */
(function(){
if (window.__hpI18n) return; window.__hpI18n = true;
const LANGS = ['en','fr','es','pt','de'];
const NAMES = {en:'English',fr:'Français',es:'Español',pt:'Português',de:'Deutsch'};
const IDX = {fr:0,es:1,pt:2,de:3};
const KEY = 'hp_lang';

/* pages that have their own static version in /fr/ /es/ /pt/ /de/ */
const STATIC = ['studio.html'];

const D = {
/* ---------- header, menu, footer, promo bar ---------- */
"Browse":["Explorer","Explorar","Explorar","Entdecken"],
"Library":["Bibliothèque","Biblioteca","Biblioteca","Bibliothek"],
"Sign In":["Connexion","Entrar","Entrar","Anmelden"],
"Sign Up":["Inscription","Registrarse","Criar conta","Registrieren"],
"Beats":["Instrus","Beats","Beats","Beats"],
"Studio Records":["Studio Records","Studio Records","Studio Records","Studio Records"],
"Plugins":["Plugins","Plugins","Plugins","Plugins"],
"Drum Kits":["Drum Kits","Drum Kits","Drum Kits","Drum Kits"],
"Streetwear":["Streetwear","Streetwear","Streetwear","Streetwear"],
"Mastering":["Mastering","Mastering","Masterização","Mastering"],
"Video Studio":["Studio vidéo","Estudio de vídeo","Estúdio de vídeo","Videostudio"],
"Info":["Infos","Info","Info","Info"],
"Contact":["Contact","Contacto","Contato","Kontakt"],
"My Account":["Mon compte","Mi cuenta","Minha conta","Mein Konto"],
"License":["Licence","Licencia","Licença","Lizenz"],
"Quick links":["Liens rapides","Enlaces rápidos","Links rápidos","Schnelllinks"],
"Legal":["Mentions légales","Legal","Legal","Rechtliches"],
"For Creators":["Pour les créateurs","Para creadores","Para criadores","Für Creator"],
"Follow DJBILBOX":["Suivre DJBILBOX","Seguir a DJBILBOX","Seguir DJBILBOX","DJBILBOX folgen"],
"Home":["Accueil","Inicio","Início","Startseite"],
"Discography":["Discographie","Discografía","Discografia","Diskografie"],
"Playlists":["Playlists","Playlists","Playlists","Playlists"],
"News":["Actus","Noticias","Novidades","News"],
"Reviews":["Avis","Opiniones","Avaliações","Bewertungen"],
"Blog":["Blog","Blog","Blog","Blog"],
"Studio Setup":["Matériel du studio","Equipo del estudio","Equipamento do estúdio","Studio-Setup"],
"Services":["Services","Servicios","Serviços","Leistungen"],
"Beat Licensing":["Licences des instrus","Licencias de beats","Licenças de beats","Beat-Lizenzen"],
"Pricing":["Tarifs","Precios","Preços","Preise"],
"Free Beats":["Instrus gratuites","Beats gratis","Beats grátis","Kostenlose Beats"],
"Free VST Plugins":["Plugins VST gratuits","Plugins VST gratis","Plugins VST grátis","Kostenlose VST-Plugins"],
"Beats — licenses from $29":["Instrus — licences dès 29 $","Beats — licencias desde 29 $","Beats — licenças a partir de 29 $","Beats — Lizenzen ab 29 $"],
"Academy (Discord)":["Académie (Discord)","Academia (Discord)","Academia (Discord)","Akademie (Discord)"],
"Mix & Master":["Mix & Master","Mezcla y máster","Mix e master","Mix & Master"],
"Global Cities":["Villes du monde","Ciudades del mundo","Cidades do mundo","Städte weltweit"],
"© 2026 DJBILBOX BEATS — All rights reserved.":["© 2026 DJBILBOX BEATS — Tous droits réservés.","© 2026 DJBILBOX BEATS — Todos los derechos reservados.","© 2026 DJBILBOX BEATS — Todos os direitos reservados.","© 2026 DJBILBOX BEATS — Alle Rechte vorbehalten."],
"🔥 800+ Beats":["🔥 800+ instrus","🔥 800+ beats","🔥 800+ beats","🔥 800+ Beats"],
"— Free tagged MP3 · licenses from $29.":["— MP3 taguée gratuite · licences dès 29 $.","— MP3 con tag gratis · licencias desde 29 $.","— MP3 com tag grátis · licenças a partir de 29 $.","— Kostenlose getaggte MP3 · Lizenzen ab 29 $."],
"Browse beats":["Voir les instrus","Ver los beats","Ver os beats","Beats ansehen"],
"Cart":["Panier","Carrito","Carrinho","Warenkorb"],
"Menu":["Menu","Menú","Menu","Menü"],
"Your cart":["Ton panier","Tu carrito","Seu carrinho","Dein Warenkorb"],
"Total":["Total","Total","Total","Gesamt"],
"FREE":["GRATUIT","GRATIS","GRÁTIS","GRATIS"],
"View full cart":["Voir le panier","Ver el carrito","Ver o carrinho","Warenkorb ansehen"],
"Checkout on Gumroad":["Payer sur Gumroad","Pagar en Gumroad","Pagar no Gumroad","Bei Gumroad bezahlen"],
"Clear cart":["Vider le panier","Vaciar el carrito","Esvaziar o carrinho","Warenkorb leeren"],
"Your cart is empty.":["Ton panier est vide.","Tu carrito está vacío.","Seu carrinho está vazio.","Dein Warenkorb ist leer."],
"Add sample kits, VST plugins or packs.":["Ajoute des kits, des plugins VST ou des packs.","Añade kits, plugins VST o packs.","Adicione kits, plugins VST ou packs.","Füge Kits, VST-Plugins oder Packs hinzu."],
"Buy":["Acheter","Comprar","Comprar","Kaufen"],
"Free for profit":["Libre pour usage commercial","Libre para uso comercial","Livre para uso comercial","Frei für kommerzielle Nutzung"],
"Free For Profit":["Libre pour usage commercial","Libre para uso comercial","Livre para uso comercial","Frei für kommerzielle Nutzung"],
"No beats match this filter.":["Aucune instru ne correspond à ce filtre.","Ningún beat coincide con este filtro.","Nenhum beat corresponde a este filtro.","Kein Beat passt zu diesem Filter."],
"FREE demo available":["Démo GRATUITE disponible","Demo GRATIS disponible","Demo GRÁTIS disponível","Kostenlose Demo verfügbar"],
"SALE":["PROMO","OFERTA","PROMOÇÃO","SALE"],
"Sale":["Promo","Oferta","Promoção","Sale"],
"Free demo":["Démo gratuite","Demo gratis","Demo grátis","Kostenlose Demo"],
"View details":["Voir le détail","Ver detalles","Ver detalhes","Details ansehen"],
"Add":["Ajouter","Añadir","Adicionar","Hinzufügen"],
"No packs in this category.":["Aucun pack dans cette catégorie.","No hay packs en esta categoría.","Nenhum pack nesta categoria.","Keine Packs in dieser Kategorie."],
"Featured":["En avant","Destacados","Destaques","Empfohlen"],
"Price: Low → High":["Prix : croissant","Precio: menor a mayor","Preço: menor para maior","Preis: aufsteigend"],
"Price: High → Low":["Prix : décroissant","Precio: mayor a menor","Preço: maior para menor","Preis: absteigend"],
"Free first":["Gratuits d'abord","Gratis primero","Grátis primeiro","Kostenlose zuerst"],
"On request":["Sur demande","A petición","Sob pedido","Auf Anfrage"],
"Coming soon":["Bientôt","Próximamente","Em breve","Demnächst"],
"Demo":["Démo","Demo","Demo","Demo"],
"Notify":["Me prévenir","Avisarme","Avisar-me","Benachrichtigen"],
"Get Free":["Obtenir gratuitement","Conseguir gratis","Obter grátis","Gratis holen"],
"Free":["Gratuit","Gratis","Grátis","Gratis"],
"New":["Nouveau","Nuevo","Novo","Neu"],
"NEW":["NOUVEAU","NUEVO","NOVO","NEU"],
"Video":["Vidéo","Vídeo","Vídeo","Video"],
"Play":["Lecture","Reproducir","Reproduzir","Abspielen"],
"Store":["Boutique","Tienda","Loja","Shop"],
"All products":["Tous les produits","Todos los productos","Todos os produtos","Alle Produkte"],
"Language":["Langue","Idioma","Idioma","Sprache"],

/* ---------- home ---------- */
"HUMPIRE — by DJBILBOX BEATS · Worldwide":["HUMPIRE — par DJBILBOX BEATS · Partout dans le monde","HUMPIRE — por DJBILBOX BEATS · En todo el mundo","HUMPIRE — por DJBILBOX BEATS · No mundo todo","HUMPIRE — von DJBILBOX BEATS · Weltweit"],
"Beatmaker, producer":["Beatmaker, producteur","Beatmaker, productor","Beatmaker, produtor","Beatmaker, Produzent"],
"plugin maker.":["créateur de plugins.","creador de plugins.","criador de plugins.","Plugin-Entwickler."],
"HUMPIRE is the store and creative hub of DJBILBOX BEATS — a beatmaker and producer serving artists worldwide. Instrumentals, VST plugins, drum kits, hands-on mastering, and AI-powered videos, all built by one producer. 1000+ free-for-profit instrumentals, instant licenses. One studio, one vision.":["HUMPIRE, c'est la boutique et le QG créatif de DJBILBOX BEATS — beatmaker et producteur au service des artistes du monde entier. Instrus, plugins VST, drum kits, mastering fait main et vidéos boostées à l'IA, le tout créé par un seul producteur. Plus de 1000 instrus libres pour usage commercial, licences immédiates. Un studio, une vision.","HUMPIRE es la tienda y el centro creativo de DJBILBOX BEATS — beatmaker y productor al servicio de artistas de todo el mundo. Instrumentales, plugins VST, drum kits, mastering a mano y vídeos con IA, todo hecho por un solo productor. Más de 1000 instrumentales libres para uso comercial, licencias al instante. Un estudio, una visión.","HUMPIRE é a loja e o centro criativo de DJBILBOX BEATS — beatmaker e produtor a serviço de artistas do mundo todo. Instrumentais, plugins VST, drum kits, masterização feita à mão e vídeos com IA, tudo feito por um único produtor. Mais de 1000 instrumentais livres para uso comercial, licenças na hora. Um estúdio, uma visão.","HUMPIRE ist der Shop und das kreative Zentrum von DJBILBOX BEATS — Beatmaker und Produzent für Artists weltweit. Instrumentals, VST-Plugins, Drum Kits, handgemachtes Mastering und KI-Videos, alles von einem Produzenten. Über 1000 Instrumentals frei für kommerzielle Nutzung, Lizenzen sofort. Ein Studio, eine Vision."],
"Listen to beats":["Écouter les instrus","Escuchar los beats","Ouvir os beats","Beats anhören"],
"Create free account":["Créer un compte gratuit","Crear una cuenta gratis","Criar conta grátis","Kostenloses Konto erstellen"],
"🎧 About the beatmaker":["🎧 Le beatmaker","🎧 Sobre el beatmaker","🎧 Sobre o beatmaker","🎧 Über den Beatmaker"],
"🦊 NEW: FENNEC":["🦊 NOUVEAU : FENNEC","🦊 NUEVO: FENNEC","🦊 NOVO: FENNEC","🦊 NEU: FENNEC"],
"🎄 Soon: QUEEN OF XMAS & TALKBOX":["🎄 Bientôt : QUEEN OF XMAS & TALKBOX","🎄 Pronto: QUEEN OF XMAS & TALKBOX","🎄 Em breve: QUEEN OF XMAS & TALKBOX","🎄 Bald: QUEEN OF XMAS & TALKBOX"],
"🎚️ Mastering":["🎚️ Mastering","🎚️ Mastering","🎚️ Masterização","🎚️ Mastering"],
"🎬 Video Studio":["🎬 Studio vidéo","🎬 Estudio de vídeo","🎬 Estúdio de vídeo","🎬 Videostudio"],
"YouTube Views":["Vues YouTube","Vistas en YouTube","Visualizações no YouTube","YouTube-Aufrufe"],
"Spotify Tracks":["Titres Spotify","Temas en Spotify","Faixas no Spotify","Spotify-Tracks"],
"Videos":["Vidéos","Vídeos","Vídeos","Videos"],
"Gumroad Sales":["Ventes Gumroad","Ventas en Gumroad","Vendas no Gumroad","Gumroad-Verkäufe"],
"Follow & stream:":["Suivre & écouter :","Sigue y escucha:","Siga e ouça:","Folgen & streamen:"],
"The oriental orchestra in your DAW: 10 real instruments, 330 presets, real quarter tones and 5 skins. Oud, saz, qanun, violin, ney, mizmar, darbuka and more. No Kontakt needed.":["L'orchestre oriental dans ton DAW : 10 vrais instruments, 330 presets, de vrais quarts de ton et 5 skins. Oud, saz, qanun, violon, ney, mizmar, darbouka et plus. Sans Kontakt.","La orquesta oriental en tu DAW: 10 instrumentos reales, 330 presets, cuartos de tono reales y 5 skins. Oud, saz, qanun, violín, ney, mizmar, darbuka y más. Sin Kontakt.","A orquestra oriental na sua DAW: 10 instrumentos reais, 330 presets, quartos de tom reais e 5 skins. Oud, saz, qanun, violino, ney, mizmar, darbuka e mais. Sem Kontakt.","Das orientalische Orchester in deiner DAW: 10 echte Instrumente, 330 Presets, echte Vierteltöne und 5 Skins. Oud, Saz, Qanun, Violine, Ney, Mizmar, Darbuka und mehr. Kein Kontakt nötig."],
"10 REAL INSTRUMENTS":["10 VRAIS INSTRUMENTS","10 INSTRUMENTOS REALES","10 INSTRUMENTOS REAIS","10 ECHTE INSTRUMENTE"],
"QUARTER TONES":["QUARTS DE TON","CUARTOS DE TONO","QUARTOS DE TOM","VIERTELTÖNE"],
"Get FENNEC":["Obtenir FENNEC","Conseguir FENNEC","Obter FENNEC","FENNEC holen"],
"See all features":["Toutes les fonctions","Ver todas las funciones","Ver todos os recursos","Alle Funktionen"],
"COMING SOON":["BIENTÔT","PRÓXIMAMENTE","EM BREVE","DEMNÄCHST"],
"Two new premium synths on the way":["Deux nouveaux synthés premium arrivent","Dos nuevos sintes premium en camino","Dois novos sintetizadores premium a caminho","Zwei neue Premium-Synths unterwegs"],
"Photo-real animated interfaces, 70 presets each, VST3 / AU for Windows & Mac.":["Interfaces animées photo-réalistes, 70 presets chacun, VST3 / AU pour Windows & Mac.","Interfaces animadas fotorrealistas, 70 presets cada uno, VST3 / AU para Windows y Mac.","Interfaces animadas fotorrealistas, 70 presets cada, VST3 / AU para Windows e Mac.","Fotorealistische animierte Oberflächen, je 70 Presets, VST3 / AU für Windows & Mac."],
"Try them right here before release.":["Essaie-les ici avant la sortie.","Pruébalos aquí antes del lanzamiento.","Experimente aqui antes do lançamento.","Probier sie hier vor dem Release aus."],
"— diva voice, sleigh bells, church bells: real recorded instruments for your holiday beats.":["— voix de diva, grelots, cloches d'église : de vrais instruments enregistrés pour tes instrus de Noël.","— voz de diva, cascabeles, campanas de iglesia: instrumentos reales grabados para tus beats navideños.","— voz de diva, guizos, sinos de igreja: instrumentos reais gravados para seus beats de Natal.","— Diva-Stimme, Schlittenglocken, Kirchenglocken: echt aufgenommene Instrumente für deine Weihnachtsbeats."],
"— ooh-wah, \"yeah\", vocoder and whistle leads for G-Funk and R&B, played from the keys.":["— ooh-wah, « yeah », vocodeur et leads sifflés pour le G-Funk et le R&B, joués au clavier.","— ooh-wah, \"yeah\", vocoder y leads silbados para G-Funk y R&B, tocados desde el teclado.","— ooh-wah, \"yeah\", vocoder e leads assobiados para G-Funk e R&B, tocados no teclado.","— Ooh-wah, „yeah“, Vocoder und Pfeif-Leads für G-Funk und R&B, direkt von der Tastatur."],
"Get notified on YouTube":["Être prévenu sur YouTube","Recibir aviso en YouTube","Ser avisado no YouTube","Auf YouTube benachrichtigen lassen"],
"Downloads":["Téléchargements","Descargas","Downloads","Downloads"],
"Videos Published":["Vidéos publiées","Vídeos publicados","Vídeos publicados","Veröffentlichte Videos"],
"🎁 100% free":["🎁 100 % gratuit","🎁 100 % gratis","🎁 100 % grátis","🎁 100 % gratis"],
"Free VST plugins":["Plugins VST gratuits","Plugins VST gratis","Plugins VST grátis","Kostenlose VST-Plugins"],
"Download them for free (pay what you want) and use them in your beats.":["Télécharge-les gratuitement (prix libre) et utilise-les dans tes instrus.","Descárgalos gratis (paga lo que quieras) y úsalos en tus beats.","Baixe grátis (pague quanto quiser) e use nos seus beats.","Lade sie kostenlos herunter (zahl, was du willst) und nutze sie in deinen Beats."],
"See all free plugins":["Voir tous les plugins gratuits","Ver todos los plugins gratis","Ver todos os plugins grátis","Alle kostenlosen Plugins"],
"Free Download":["Téléchargement gratuit","Descarga gratis","Download grátis","Kostenloser Download"],
"New Release":["Nouvelle sortie","Nuevo lanzamiento","Novo lançamento","Neuerscheinung"],
"VST Plugins":["Plugins VST","Plugins VST","Plugins VST","VST-Plugins"],
"1000+ Free Beats":["1000+ instrus gratuites","1000+ beats gratis","1000+ beats grátis","1000+ kostenlose Beats"],
"Zombies Game":["Jeu Zombies","Juego Zombies","Jogo Zombies","Zombies-Spiel"],
"★ Flagship plug-in":["★ Plugin phare","★ Plugin estrella","★ Plugin principal","★ Flaggschiff-Plugin"],
"Best VST for oriental sound":["Le meilleur VST pour le son oriental","El mejor VST para el sonido oriental","O melhor VST para som oriental","Das beste VST für orientalischen Sound"],
"280+ authentic oriental instruments — oud, qanun, ney, saz, darbuka — driven by a real Maqam quarter-tone engine. VST3 & AU, Windows & Mac.":["Plus de 280 instruments orientaux authentiques — oud, qanun, ney, saz, darbouka — pilotés par un vrai moteur maqam en quarts de ton. VST3 & AU, Windows & Mac.","Más de 280 instrumentos orientales auténticos — oud, qanun, ney, saz, darbuka — con un motor maqam real de cuartos de tono. VST3 y AU, Windows y Mac.","Mais de 280 instrumentos orientais autênticos — oud, qanun, ney, saz, darbuka — com um motor maqam real de quartos de tom. VST3 e AU, Windows e Mac.","Über 280 authentische orientalische Instrumente — Oud, Qanun, Ney, Saz, Darbuka — mit echter Maqam-Vierteltonengine. VST3 & AU, Windows & Mac."],
"Discover it":["Le découvrir","Descúbrelo","Descubra","Entdecken"],
"✅ Free":["✅ Gratuit","✅ Gratis","✅ Grátis","✅ Gratis"],
"2Pac, Snoop Dogg & The Game vibes · Hip-Hop loops · Free download":["Ambiance 2Pac, Snoop Dogg & The Game · Boucles hip-hop · Téléchargement gratuit","Rollo 2Pac, Snoop Dogg y The Game · Loops hip-hop · Descarga gratis","Vibe 2Pac, Snoop Dogg e The Game · Loops hip-hop · Download grátis","2Pac-, Snoop-Dogg- & The-Game-Vibes · Hip-Hop-Loops · Kostenloser Download"],
"🎹 Funk Collaboration":["🎹 Collaboration funk","🎹 Colaboración funk","🎹 Colaboração funk","🎹 Funk-Kollaboration"],
"By DABEULL, DOGG MASTER, KAZZEY, MOFAK · Professional kit · Free download":["Par DABEULL, DOGG MASTER, KAZZEY, MOFAK · Kit pro · Téléchargement gratuit","Por DABEULL, DOGG MASTER, KAZZEY, MOFAK · Kit profesional · Descarga gratis","Por DABEULL, DOGG MASTER, KAZZEY, MOFAK · Kit profissional · Download grátis","Von DABEULL, DOGG MASTER, KAZZEY, MOFAK · Profi-Kit · Kostenloser Download"],
"🆕 Free VST Synth":["🆕 Synthé VST gratuit","🆕 Sinte VST gratis","🆕 Sintetizador VST grátis","🆕 Kostenloser VST-Synth"],
"Synthwave polysynth · 6 factory presets · Completely FREE":["Polysynthé synthwave · 6 presets d'usine · Entièrement GRATUIT","Polisinte synthwave · 6 presets de fábrica · Totalmente GRATIS","Polissintetizador synthwave · 6 presets de fábrica · Totalmente GRÁTIS","Synthwave-Polysynth · 6 Werkspresets · Komplett GRATIS"],
"Studio-grade instruments & effects":["Instruments & effets de niveau studio","Instrumentos y efectos de calidad de estudio","Instrumentos e efeitos de nível de estúdio","Instrumente & Effekte in Studioqualität"],
"All VST Plugins":["Tous les plugins VST","Todos los plugins VST","Todos os plugins VST","Alle VST-Plugins"],
"View all →":["Tout voir →","Ver todo →","Ver tudo →","Alle ansehen →"],
"Explore":["Explorer","Explorar","Explorar","Entdecken"],
"The Catalog":["Le catalogue","El catálogo","O catálogo","Der Katalog"],
"🌴 Enter DJBILBOX CITY":["🌴 Entrer dans DJBILBOX CITY","🌴 Entra en DJBILBOX CITY","🌴 Entre em DJBILBOX CITY","🌴 Willkommen in DJBILBOX CITY"],
"Walk the neon street — every shop opens a part of the studio":["Balade-toi dans la rue néon — chaque boutique ouvre une partie du studio","Recorre la calle de neón — cada tienda abre una parte del estudio","Passeie pela rua neon — cada loja abre uma parte do estúdio","Lauf durch die Neon-Straße — jeder Laden öffnet einen Teil des Studios"],
"280+ instruments · Maqam engine · free demo":["280+ instruments · moteur maqam · démo gratuite","280+ instrumentos · motor maqam · demo gratis","280+ instrumentos · motor maqam · demo grátis","280+ Instrumente · Maqam-Engine · kostenlose Demo"],
"Music videos & social edits — made with Filmora":["Clips & montages réseaux — faits avec Filmora","Videoclips y edits para redes — hechos con Filmora","Clipes e edições para redes — feitos com Filmora","Musikvideos & Social-Edits — gemacht mit Filmora"],
"10,000+ beats by region & genre worldwide":["10 000+ instrus par région et par style","10 000+ beats por región y género","10 000+ beats por região e gênero","10.000+ Beats nach Region & Genre"],
"Pro sample packs · WAV":["Sample packs pro · WAV","Sample packs pro · WAV","Sample packs pro · WAV","Profi-Sample-Packs · WAV"],
"📺 Live on Twitch":["📺 En direct sur Twitch","📺 En directo en Twitch","📺 Ao vivo na Twitch","📺 Live auf Twitch"],
"Beat production · VST demos · Tutorials":["Production d'instrus · démos VST · tutos","Producción de beats · demos VST · tutoriales","Produção de beats · demos VST · tutoriais","Beat-Produktion · VST-Demos · Tutorials"],
"Academy":["Académie","Academia","Academia","Akademie"],
"Training: AI · Beatmaking · VST · Twitch — via Discord":["Formations : IA · beatmaking · VST · Twitch — sur Discord","Formación: IA · beatmaking · VST · Twitch — en Discord","Formação: IA · beatmaking · VST · Twitch — no Discord","Kurse: KI · Beatmaking · VST · Twitch — über Discord"],
"Mix · Master · Video · Branding":["Mix · master · vidéo · branding","Mezcla · máster · vídeo · branding","Mix · master · vídeo · branding","Mix · Master · Video · Branding"],
"Albums & EPs on Spotify":["Albums & EP sur Spotify","Álbumes y EP en Spotify","Álbuns e EPs no Spotify","Alben & EPs auf Spotify"],
"Free · Basic · Premium · Exclusive":["Gratuite · Basic · Premium · Exclusive","Gratis · Basic · Premium · Exclusiva","Grátis · Basic · Premium · Exclusiva","Gratis · Basic · Premium · Exklusiv"],
"What artists say":["Ce que disent les artistes","Lo que dicen los artistas","O que os artistas dizem","Was Artists sagen"],
"New drops & exclusive offers":["Nouveautés & offres exclusives","Novedades y ofertas exclusivas","Novidades e ofertas exclusivas","Neuheiten & exklusive Angebote"],
"Hardware, DAW & production gear":["Matériel, DAW & équipement de prod","Hardware, DAW y equipo de producción","Hardware, DAW e equipamento de produção","Hardware, DAW & Produktions-Equipment"],
"Free beats for 300+ cities worldwide":["Instrus gratuites pour 300+ villes du monde","Beats gratis para 300+ ciudades","Beats grátis para 300+ cidades","Kostenlose Beats für 300+ Städte weltweit"],
"Free 16-bit game · 10 cities · play now":["Jeu 16 bits gratuit · 10 villes · joue maintenant","Juego 16 bits gratis · 10 ciudades · juega ya","Jogo 16 bits grátis · 10 cidades · jogue agora","Kostenloses 16-Bit-Spiel · 10 Städte · jetzt spielen"],
"Who is DJBILBOX BEATS?":["Qui est DJBILBOX BEATS ?","¿Quién es DJBILBOX BEATS?","Quem é DJBILBOX BEATS?","Wer ist DJBILBOX BEATS?"],
"Pro sound,":["Un son pro,","Sonido pro,","Som profissional,","Profi-Sound,"],
"zero gatekeeping.":["accessible à tous.","sin barreras.","sem barreiras.","ohne Hürden."],
"Behind DJBILBOX BEATS is":["Derrière DJBILBOX BEATS, il y a","Detrás de DJBILBOX BEATS está","Por trás de DJBILBOX BEATS está","Hinter DJBILBOX BEATS steht"],
"— producer, beatmaker and sound designer from Toulouse. For years he's been shaping a signature sound across":["— producteur, beatmaker et sound designer toulousain. Depuis des années, il façonne un son signature entre","— productor, beatmaker y diseñador de sonido de Toulouse. Desde hace años crea un sonido propio entre","— produtor, beatmaker e sound designer de Toulouse. Há anos ele molda um som próprio entre","— Produzent, Beatmaker und Sounddesigner aus Toulouse. Seit Jahren prägt er einen eigenen Sound zwischen"],
"Oriental, Trap, Drill, G-Funk, Afrotrap & House":["oriental, trap, drill, G-Funk, afrotrap & house","oriental, trap, drill, G-Funk, afrotrap y house","oriental, trap, drill, G-Funk, afrotrap e house","Oriental, Trap, Drill, G-Funk, Afrotrap & House"],
", and sharing it with artists everywhere.":[", et le partage avec les artistes du monde entier.",", y lo comparte con artistas de todas partes.",", e o compartilha com artistas do mundo todo.",", und teilt ihn mit Artists überall."],
"The mission is simple: give every artist":["La mission est simple : donner à chaque artiste","La misión es simple: dar a cada artista","A missão é simples: dar a cada artista","Die Mission ist einfach: jedem Artist"],
"studio-grade tools without the studio price":["des outils de studio sans le prix du studio","herramientas de estudio sin el precio de un estudio","ferramentas de estúdio sem o preço de estúdio","Studio-Tools ohne Studiopreis zu geben"],
". Beats are free-for-profit with instant licensing, the VST plugins bring real signature sounds to any DAW, and the drum kits are built from the very sounds used on his own records. 🐪🎧":[". Les instrus sont libres pour usage commercial avec licence immédiate, les plugins VST apportent de vrais sons signature dans n'importe quel DAW, et les drum kits sont faits avec les sons de ses propres morceaux. 🐪🎧",". Los beats son libres para uso comercial con licencia inmediata, los plugins VST llevan sonidos propios a cualquier DAW y los drum kits están hechos con los mismos sonidos de sus temas. 🐪🎧",". Os beats são livres para uso comercial com licença imediata, os plugins VST levam sons próprios a qualquer DAW e os drum kits são feitos com os mesmos sons das suas faixas. 🐪🎧",". Die Beats sind frei für kommerzielle Nutzung mit sofortiger Lizenz, die VST-Plugins bringen echte Signature-Sounds in jede DAW, und die Drum Kits bestehen aus genau den Sounds seiner eigenen Tracks. 🐪🎧"],
"Spotify tracks":["Titres Spotify","Temas en Spotify","Faixas no Spotify","Spotify-Tracks"],
"YouTube videos":["Vidéos YouTube","Vídeos de YouTube","Vídeos no YouTube","YouTube-Videos"],
"Views":["Vues","Vistas","Visualizações","Aufrufe"],
"Retro synth wave instrument for Win & Mac. Instant download on Gumroad.":["Instrument synthwave rétro pour Win & Mac. Téléchargement immédiat sur Gumroad.","Instrumento synthwave retro para Win y Mac. Descarga inmediata en Gumroad.","Instrumento synthwave retrô para Win e Mac. Download imediato no Gumroad.","Retro-Synthwave-Instrument für Win & Mac. Sofort-Download bei Gumroad."],
"Get it":["L'obtenir","Conseguirlo","Obter","Holen"],
"✅ 100% Free":["✅ 100 % gratuit","✅ 100 % gratis","✅ 100 % grátis","✅ 100 % gratis"],
"A dual-oscillator synthwave polysynth — detune, sub, PWM, resonant filter with envelope, LFO, chorus, delay and reverb. Six presets, no licence key, no email wall.":["Un polysynthé synthwave à deux oscillateurs — désaccord, sub, PWM, filtre résonant avec enveloppe, LFO, chorus, delay et reverb. Six presets, sans clé de licence, sans e-mail obligatoire.","Un polisinte synthwave de dos osciladores — detune, sub, PWM, filtro resonante con envolvente, LFO, chorus, delay y reverb. Seis presets, sin clave de licencia, sin pedir e-mail.","Um polissintetizador synthwave de dois osciladores — detune, sub, PWM, filtro ressonante com envelope, LFO, chorus, delay e reverb. Seis presets, sem chave de licença, sem pedir e-mail.","Ein Synthwave-Polysynth mit zwei Oszillatoren — Detune, Sub, PWM, resonanter Filter mit Hüllkurve, LFO, Chorus, Delay und Reverb. Sechs Presets, kein Lizenzschlüssel, keine E-Mail-Pflicht."],
"Download free":["Télécharger gratuitement","Descargar gratis","Baixar grátis","Kostenlos laden"],
"Details":["Détails","Detalles","Detalhes","Details"],
"🎬 VIDEO STUDIO":["🎬 STUDIO VIDÉO","🎬 ESTUDIO DE VÍDEO","🎬 ESTÚDIO DE VÍDEO","🎬 VIDEOSTUDIO"],
"Your beat, seen everywhere.":["Ton son, vu partout.","Tu beat, visto en todas partes.","Seu beat, visto em todo lugar.","Dein Beat, überall gesehen."],
"Music videos, beat visualizers, lyric videos and vertical cut-downs for Shorts, Reels & TikTok — cut to your BPM and exported in every format. Edited in-house with":["Clips, visualiseurs, lyric videos et formats verticaux pour Shorts, Reels & TikTok — montés sur ton BPM et exportés dans tous les formats. Montés en interne avec","Videoclips, visualizadores, lyric videos y cortes verticales para Shorts, Reels y TikTok — editados a tu BPM y exportados en todos los formatos. Editados en casa con","Clipes, visualizadores, lyric videos e cortes verticais para Shorts, Reels e TikTok — editados no seu BPM e exportados em todos os formatos. Editados internamente com","Musikvideos, Visualizer, Lyric-Videos und Hochformat-Cuts für Shorts, Reels & TikTok — auf dein BPM geschnitten und in jedem Format exportiert. Inhouse geschnitten mit"],
"Open Video Studio":["Ouvrir le studio vidéo","Abrir el estudio de vídeo","Abrir o estúdio de vídeo","Videostudio öffnen"],
"📺 LIVE · Production Streams":["📺 LIVE · Streams de production","📺 EN DIRECTO · Streams de producción","📺 AO VIVO · Lives de produção","📺 LIVE · Produktions-Streams"],
"Watch on Twitch":["Regarder sur Twitch","Ver en Twitch","Assistir na Twitch","Auf Twitch ansehen"],
"Join live beat production streams, VST tutorials, and music production tips. Monday, Wednesday, Friday & Sunday at 8:00 PM CET.":["Rejoins les lives de production d'instrus, tutos VST et astuces de prod. Lundi, mercredi, vendredi et dimanche à 20 h (heure de Paris).","Únete a los directos de producción de beats, tutoriales VST y consejos de producción. Lunes, miércoles, viernes y domingo a las 20:00 CET.","Participe das lives de produção de beats, tutoriais VST e dicas de produção. Segunda, quarta, sexta e domingo às 20:00 CET.","Sei dabei bei Live-Beat-Produktion, VST-Tutorials und Produktionstipps. Montag, Mittwoch, Freitag & Sonntag um 20:00 Uhr MEZ."],
"✓ Live Beat Creation ✓ VST Demos ✓ Production Tips":["✓ Création d'instrus en direct ✓ Démos VST ✓ Astuces de prod","✓ Creación de beats en directo ✓ Demos VST ✓ Consejos de producción","✓ Criação de beats ao vivo ✓ Demos VST ✓ Dicas de produção","✓ Live-Beatproduktion ✓ VST-Demos ✓ Produktionstipps"],
"Watch Live":["Voir le live","Ver en directo","Assistir ao vivo","Live ansehen"],
"Explore our content: tutorials, production tips, sample pack breakdowns, and professional media kit.":["Découvre nos contenus : tutos, astuces de prod, analyses de sample packs et media kit pro.","Explora nuestro contenido: tutoriales, consejos de producción, análisis de sample packs y media kit profesional.","Explore nosso conteúdo: tutoriais, dicas de produção, análises de sample packs e media kit profissional.","Entdecke unsere Inhalte: Tutorials, Produktionstipps, Sample-Pack-Breakdowns und ein professionelles Media-Kit."],
"Articles":["Articles","Artículos","Artigos","Artikel"],
"🎮 FREE GAME · 16-Bit Action":["🎮 JEU GRATUIT · Action 16 bits","🎮 JUEGO GRATIS · Acción 16 bits","🎮 JOGO GRÁTIS · Ação 16 bits","🎮 KOSTENLOSES SPIEL · 16-Bit-Action"],
"Urban beat'em up with 10 cities, boss battles, and progressive weapons. Play now in your browser — no download required.":["Beat'em up urbain avec 10 villes, combats de boss et armes à débloquer. Joue directement dans ton navigateur — rien à télécharger.","Beat'em up urbano con 10 ciudades, jefes finales y armas que mejoran. Juega ya en tu navegador — sin descargas.","Beat'em up urbano com 10 cidades, chefões e armas evolutivas. Jogue agora no navegador — sem download.","Urbanes Beat'em-up mit 10 Städten, Bosskämpfen und aufrüstbaren Waffen. Direkt im Browser spielen — kein Download."],
"✓ 10 Cities ✓ Boss Battles ✓ Upgradeable Weapons":["✓ 10 villes ✓ Combats de boss ✓ Armes améliorables","✓ 10 ciudades ✓ Jefes finales ✓ Armas mejorables","✓ 10 cidades ✓ Chefões ✓ Armas evolutivas","✓ 10 Städte ✓ Bosskämpfe ✓ Aufrüstbare Waffen"],
"Play Now":["Jouer","Jugar ya","Jogar agora","Jetzt spielen"],
"Stay in the loop":["Reste informé","Mantente al día","Fique por dentro","Bleib auf dem Laufenden"],
"New beats, free drops & exclusive offers — straight to your inbox.":["Nouvelles instrus, cadeaux & offres exclusives — directement dans ta boîte mail.","Nuevos beats, regalos y ofertas exclusivas — directo a tu correo.","Novos beats, brindes e ofertas exclusivas — direto no seu e-mail.","Neue Beats, Gratis-Drops & exklusive Angebote — direkt in dein Postfach."],
"your@email.com":["ton@email.com","tu@email.com","seu@email.com","deine@email.com"],
"Subscribe":["S'abonner","Suscribirse","Inscrever-se","Abonnieren"],
"All HUMPIRE products: free VST plugins, synths, effects, drum kits & sample packs":["Tous les produits HUMPIRE : plugins VST gratuits, synthés, effets, drum kits & sample packs","Todos los productos HUMPIRE: plugins VST gratis, sintes, efectos, drum kits y sample packs","Todos os produtos HUMPIRE: plugins VST grátis, sintetizadores, efeitos, drum kits e sample packs","Alle HUMPIRE-Produkte: kostenlose VST-Plugins, Synths, Effekte, Drum Kits & Sample Packs"],

/* ---------- beats ---------- */
"DJBILBOX BEATS · Instrumentals":["DJBILBOX BEATS · Instrus","DJBILBOX BEATS · Instrumentales","DJBILBOX BEATS · Instrumentais","DJBILBOX BEATS · Instrumentals"],
"Beats that move.":["Des instrus qui bougent.","Beats que mueven.","Beats que movem.","Beats, die bewegen."],
"Built for your next record.":["Faites pour ton prochain son.","Hechos para tu próximo tema.","Feitos para sua próxima faixa.","Gemacht für deinen nächsten Track."],
"Stream every beat here. Try it with the free tagged MP3, then grab a license from $29 to release your song.":["Écoute toutes les instrus ici. Essaie avec la MP3 taguée gratuite, puis prends une licence dès 29 $ pour sortir ton son.","Escucha todos los beats aquí. Pruébalo con el MP3 con tag gratis y luego compra una licencia desde 29 $ para lanzar tu tema.","Ouça todos os beats aqui. Teste com o MP3 com tag grátis e depois pegue uma licença a partir de 29 $ para lançar sua faixa.","Hör dir hier jeden Beat an. Teste ihn mit der kostenlosen getaggten MP3 und hol dir dann ab 29 $ eine Lizenz für deinen Release."],
"Licenses from $29":["Licences dès 29 $","Licencias desde 29 $","Licenças a partir de 29 $","Lizenzen ab 29 $"],
"Free tagged MP3s":["MP3 taguées gratuites","MP3 con tag gratis","MP3 com tag grátis","Kostenlose getaggte MP3s"],
"Filters":["Filtres","Filtros","Filtros","Filter"],
"Genre":["Style","Género","Gênero","Genre"],
"Search a beat, an artist, a mood…":["Cherche une instru, un artiste, une ambiance…","Busca un beat, un artista, un mood…","Busque um beat, um artista, uma vibe…","Suche einen Beat, einen Artist, eine Stimmung…"],
"Sort":["Trier","Ordenar","Ordenar","Sortieren"],
"Newest":["Plus récentes","Más recientes","Mais recentes","Neueste"],
"Shuffle":["Aléatoire","Aleatorio","Aleatório","Zufällig"],
"Load more beats":["Plus d'instrus","Cargar más beats","Carregar mais beats","Mehr Beats laden"],
"Licenses":["Licences","Licencias","Licenças","Lizenzen"],
"Press":["Appuie sur","Pulsa","Clique em","Klick auf"],
"on any beat: it opens its SoundClick page, pick your license and pay securely there. Files are delivered instantly. Credit":["sur n'importe quelle instru : sa page SoundClick s'ouvre, choisis ta licence et paie en toute sécurité. Les fichiers arrivent tout de suite. Mets le crédit","en cualquier beat: se abre su página de SoundClick, elige tu licencia y paga de forma segura. Los archivos llegan al instante. Pon el crédito","em qualquer beat: a página do SoundClick abre, escolha sua licença e pague com segurança. Os arquivos chegam na hora. Coloque o crédito","bei einem Beat: Die SoundClick-Seite öffnet sich, wähl deine Lizenz und zahl dort sicher. Die Dateien kommen sofort. Nenne"],
"on every release.":["sur chaque sortie.","en cada lanzamiento.","em cada lançamento.","bei jedem Release."],
"Tagged MP3":["MP3 taguée","MP3 con tag","MP3 com tag","Getaggte MP3"],
"Demos & practice":["Démos & entraînement","Demos y práctica","Demos e prática","Demos & Übung"],
"No streaming release":["Pas de sortie en streaming","Sin lanzamiento en streaming","Sem lançamento em streaming","Kein Streaming-Release"],
"No monetization":["Pas de monétisation","Sin monetización","Sem monetização","Keine Monetarisierung"],
"Download":["Télécharger","Descargar","Baixar","Download"],
"Untagged MP3":["MP3 sans tag","MP3 sin tag","MP3 sem tag","MP3 ohne Tag"],
"5,000 sales":["5 000 ventes","5000 ventas","5000 vendas","5.000 Verkäufe"],
"100,000 streams":["100 000 streams","100 000 streams","100 000 streams","100.000 Streams"],
"Choose a beat":["Choisir une instru","Elegir un beat","Escolher um beat","Beat wählen"],
"10,000 sales":["10 000 ventes","10 000 ventas","10 000 vendas","10.000 Verkäufe"],
"250,000 streams":["250 000 streams","250 000 streams","250 000 streams","250.000 Streams"],
"Studio-quality WAV":["WAV qualité studio","WAV calidad estudio","WAV qualidade de estúdio","WAV in Studioqualität"],
"Best value":["Meilleur rapport","Mejor valor","Melhor custo","Bester Wert"],
"Unlimited":["Illimitée","Ilimitada","Ilimitada","Unbegrenzt"],
"MP3 + WAV (+ stems when available)":["MP3 + WAV (+ pistes séparées si dispo)","MP3 + WAV (+ stems si hay)","MP3 + WAV (+ stems quando houver)","MP3 + WAV (+ Stems, falls verfügbar)"],
"Unlimited sales & streams":["Ventes & streams illimités","Ventas y streams ilimitados","Vendas e streams ilimitados","Unbegrenzte Verkäufe & Streams"],
"Unlimited videos":["Vidéos illimitées","Vídeos ilimitados","Vídeos ilimitados","Unbegrenzte Videos"],
"Radio & live shows":["Radio & concerts","Radio y conciertos","Rádio e shows","Radio & Live-Shows"],
"Exclusive":["Exclusive","Exclusiva","Exclusiva","Exklusiv"],
"The beat is yours":["L'instru est à toi","El beat es tuyo","O beat é seu","Der Beat gehört dir"],
"Removed from sale":["Retirée de la vente","Retirado de la venta","Retirado da venda","Aus dem Verkauf genommen"],
"Unlimited everything":["Tout illimité","Todo ilimitado","Tudo ilimitado","Alles unbegrenzt"],
"New beats every week on YouTube":["De nouvelles instrus chaque semaine sur YouTube","Nuevos beats cada semana en YouTube","Novos beats toda semana no YouTube","Jede Woche neue Beats auf YouTube"],
"Subscribe so you hear them first — and tell me in the comments which one you're recording on.":["Abonne-toi pour les écouter en premier — et dis-moi en commentaire sur laquelle tu poses.","Suscríbete para escucharlos primero — y dime en los comentarios en cuál estás grabando.","Inscreva-se para ouvir primeiro — e me diga nos comentários em qual você está gravando.","Abonniere, um sie zuerst zu hören — und schreib mir in die Kommentare, auf welchem du aufnimmst."],
"Watch on YouTube":["Voir sur YouTube","Ver en YouTube","Ver no YouTube","Auf YouTube ansehen"],
"Previous":["Précédent","Anterior","Anterior","Zurück"],
"Next":["Suivant","Siguiente","Próximo","Weiter"],
"Repeat":["Répéter","Repetir","Repetir","Wiederholen"],
"Seek":["Position","Posición","Posição","Position"],
"Volume":["Volume","Volumen","Volume","Lautstärke"],
"Close player":["Fermer le lecteur","Cerrar el reproductor","Fechar o player","Player schließen"],
"FREE MP3":["MP3 GRATUITE","MP3 GRATIS","MP3 GRÁTIS","GRATIS-MP3"],
"No beat matches — try another word or genre.":["Aucune instru trouvée — essaie un autre mot ou un autre style.","Ningún beat coincide — prueba otra palabra o género.","Nenhum beat encontrado — tente outra palavra ou gênero.","Kein Beat gefunden — versuch ein anderes Wort oder Genre."],
"All":["Tous","Todos","Todos","Alle"],

/* ---------- plugins / drum kits ---------- */
"Built by DJBILBOX BEATS · Toulouse":["Créé par DJBILBOX BEATS · Toulouse","Creado por DJBILBOX BEATS · Toulouse","Criado por DJBILBOX BEATS · Toulouse","Gebaut von DJBILBOX BEATS · Toulouse"],
"Real instruments.":["De vrais instruments.","Instrumentos reales.","Instrumentos reais.","Echte Instrumente."],
"Zero menus.":["Zéro menu.","Cero menús.","Zero menus.","Null Menüs."],
"Search plugins, bundles…":["Cherche un plugin, un bundle…","Busca plugins, bundles…","Busque plugins, bundles…","Plugins, Bundles suchen…"],
"Filter":["Filtrer","Filtrar","Filtrar","Filtern"],
"Christmas synth · real instruments":["Synthé de Noël · vrais instruments","Sinte navideño · instrumentos reales","Sintetizador de Natal · instrumentos reais","Weihnachts-Synth · echte Instrumente"],
"Talkbox synth · ooh-wah, vocoder":["Synthé talkbox · ooh-wah, vocodeur","Sinte talkbox · ooh-wah, vocoder","Sintetizador talkbox · ooh-wah, vocoder","Talkbox-Synth · Ooh-wah, Vocoder"],
"Nothing matches — try another word.":["Aucun résultat — essaie un autre mot.","Sin resultados — prueba otra palabra.","Nada encontrado — tente outra palavra.","Nichts gefunden — versuch ein anderes Wort."],
"Instant download":["Téléchargement immédiat","Descarga inmediata","Download imediato","Sofort-Download"],
"Delivered by Gumroad right after checkout":["Livré par Gumroad juste après le paiement","Entregado por Gumroad justo después del pago","Entregue pelo Gumroad logo após o pagamento","Von Gumroad direkt nach dem Kauf geliefert"],
"Windows & macOS":["Windows & macOS","Windows y macOS","Windows e macOS","Windows & macOS"],
"VST3 · AU · standalone on most plugins":["VST3 · AU · standalone sur la plupart des plugins","VST3 · AU · standalone en la mayoría de plugins","VST3 · AU · standalone na maioria dos plugins","VST3 · AU · Standalone bei den meisten Plugins"],
"Royalty-free":["Libre de droits","Libre de regalías","Livre de royalties","Lizenzfrei"],
"Use every sound in your own releases":["Utilise chaque son dans tes sorties","Usa cada sonido en tus lanzamientos","Use cada som nos seus lançamentos","Nutze jeden Sound in deinen Releases"],
"Try before you buy":["Essaie avant d'acheter","Prueba antes de comprar","Teste antes de comprar","Erst testen, dann kaufen"],
"Playable presets and promo videos":["Presets jouables et vidéos de démo","Presets jugables y vídeos promo","Presets tocáveis e vídeos promocionais","Spielbare Presets und Promo-Videos"],
"Sounds from my own records":["Les sons de mes propres morceaux","Sonidos de mis propios temas","Sons das minhas próprias faixas","Sounds aus meinen eigenen Tracks"],
"Straight from":["Tout droit sortis","Directo de","Direto das","Direkt aus"],
"the sessions.":["des sessions.","las sesiones.","sessões.","den Sessions."],
"Search kits, loops, bundles…":["Cherche un kit, une boucle, un bundle…","Busca kits, loops, bundles…","Busque kits, loops, bundles…","Kits, Loops, Bundles suchen…"],

/* ---------- streetwear ---------- */
"From Toulouse to Paris — the new HUMPIRE collections are here":["De Toulouse à Paris — les nouvelles collections HUMPIRE sont là","De Toulouse a París — llegan las nuevas colecciones HUMPIRE","De Toulouse a Paris — as novas coleções HUMPIRE chegaram","Von Toulouse nach Paris — die neuen HUMPIRE-Kollektionen sind da"],
"Collections":["Collections","Colecciones","Coleções","Kollektionen"],
"Summer":["Été","Verano","Verão","Sommer"],
"Women":["Femme","Mujer","Feminino","Damen"],
"Shop all":["Tout voir","Ver todo","Ver tudo","Alles shoppen"],
"Streetwear by DJBILBOX BEATS · Toulouse → Paris":["Streetwear par DJBILBOX BEATS · Toulouse → Paris","Streetwear de DJBILBOX BEATS · Toulouse → París","Streetwear por DJBILBOX BEATS · Toulouse → Paris","Streetwear von DJBILBOX BEATS · Toulouse → Paris"],
"See the collections":["Voir les collections","Ver las colecciones","Ver as coleções","Kollektionen ansehen"],
"Worldwide shipping":["Livraison dans le monde entier","Envío a todo el mundo","Envio para o mundo todo","Weltweiter Versand"],
"Printed and shipped by Fourthwall":["Imprimé et expédié par Fourthwall","Impreso y enviado por Fourthwall","Impresso e enviado pela Fourthwall","Gedruckt und versendet von Fourthwall"],
"Made to order":["Fabriqué à la commande","Hecho por encargo","Feito sob encomenda","Auf Bestellung gefertigt"],
"Every piece printed for you":["Chaque pièce imprimée pour toi","Cada prenda impresa para ti","Cada peça impressa para você","Jedes Teil für dich gedruckt"],
"Secure checkout":["Paiement sécurisé","Pago seguro","Pagamento seguro","Sichere Zahlung"],
"Card, PayPal, Apple Pay":["Carte, PayPal, Apple Pay","Tarjeta, PayPal, Apple Pay","Cartão, PayPal, Apple Pay","Karte, PayPal, Apple Pay"],
"See them worn":["Portés en vrai","Míralas puestas","Veja vestidas","Getragen ansehen"],
"Streetwear playlist":["Playlist streetwear","Playlist streetwear","Playlist streetwear","Streetwear-Playlist"],
"Close":["Fermer","Cerrar","Fechar","Schließen"],
"Shop it":["L'acheter","Comprar","Comprar","Shoppen"],
"Discover":["Découvrir","Descubrir","Descobrir","Entdecken"],
"Hoodie, oversized tees, joggers, cap and bucket hat with the purple-pink drip logo.":["Hoodie, t-shirts oversize, jogging, casquette et bob avec le logo drip violet-rose.","Sudadera, camisetas oversize, joggers, gorra y bucket hat con el logo drip morado-rosa.","Moletom, camisetas oversize, joggers, boné e bucket hat com o logo drip roxo-rosa.","Hoodie, Oversize-Tees, Jogger, Cap und Bucket Hat mit dem lila-pinken Drip-Logo."],
"Heavy artwork hoodies, the HUMPIRE pull and the balaclava.":["Hoodies à gros visuels, le pull HUMPIRE et la cagoule.","Sudaderas con mucho arte, el jersey HUMPIRE y el pasamontañas.","Moletons com arte pesada, o pulôver HUMPIRE e a balaclava.","Hoodies mit großem Artwork, der HUMPIRE-Pulli und die Sturmhaube."],
"Summer Riviera":["Riviera d'été","Riviera de verano","Riviera de verão","Sommer-Riviera"],
"Oversized tees for the terrace, the beach and the road.":["T-shirts oversize pour la terrasse, la plage et la route.","Camisetas oversize para la terraza, la playa y la carretera.","Camisetas oversize para o terraço, a praia e a estrada.","Oversize-Tees für Terrasse, Strand und Straße."],
"Hoodie, joggers, cap and balaclava built for the dunes.":["Hoodie, jogging, casquette et cagoule faits pour les dunes.","Sudadera, joggers, gorra y pasamontañas hechos para las dunas.","Moletom, joggers, boné e balaclava feitos para as dunas.","Hoodie, Jogger, Cap und Sturmhaube für die Dünen."],
"Dark artwork, sparks and smoke.":["Visuels sombres, étincelles et fumée.","Arte oscuro, chispas y humo.","Arte escura, faíscas e fumaça.","Dunkles Artwork, Funken und Rauch."],
"Cropped hoodie, oversized tees as dresses, caps and bucket hats.":["Hoodie court, t-shirts oversize en robe, casquettes et bobs.","Sudadera corta, camisetas oversize como vestido, gorras y bucket hats.","Moletom cropped, camisetas oversize como vestido, bonés e bucket hats.","Crop-Hoodie, Oversize-Tees als Kleid, Caps und Bucket Hats."],
"The Crew":["Le crew","La crew","A crew","Die Crew"],
"Every piece, worn together.":["Toutes les pièces, portées ensemble.","Todas las prendas, juntas.","Todas as peças, juntas.","Alle Teile, zusammen getragen."],
"The beatmaker behind HUMPIRE.":["Le beatmaker derrière HUMPIRE.","El beatmaker detrás de HUMPIRE.","O beatmaker por trás da HUMPIRE.","Der Beatmaker hinter HUMPIRE."],
"Neon oversized tees.":["T-shirts oversize néon.","Camisetas oversize neón.","Camisetas oversize neon.","Neon-Oversize-Tees."],
"Black essentials from the RAPFORCE38 crew.":["Les basiques noirs du crew RAPFORCE38.","Básicos negros de la crew RAPFORCE38.","Básicos pretos da crew RAPFORCE38.","Schwarze Basics der RAPFORCE38-Crew."],
"Mug & tote.":["Mug & tote bag.","Taza y tote bag.","Caneca e ecobag.","Tasse & Tote Bag."],

/* ---------- contact / cart / thank you ---------- */
"Questions about a plugin, a sample pack, your download or a licence? Collaboration or press request?":["Une question sur un plugin, un sample pack, ton téléchargement ou une licence ? Une collaboration ou une demande presse ?","¿Preguntas sobre un plugin, un sample pack, tu descarga o una licencia? ¿Colaboración o prensa?","Dúvidas sobre um plugin, um sample pack, seu download ou uma licença? Colaboração ou imprensa?","Fragen zu einem Plugin, einem Sample Pack, deinem Download oder einer Lizenz? Kollaboration oder Presseanfrage?"],
"Email:":["E-mail :","Correo:","E-mail:","E-Mail:"],
"Your purchases:":["Tes achats :","Tus compras:","Suas compras:","Deine Käufe:"],
"all downloads are in your":["tous les téléchargements sont dans ta","todas las descargas están en tu","todos os downloads estão na sua","alle Downloads sind in deiner"],
"Gumroad library":["bibliothèque Gumroad","biblioteca de Gumroad","biblioteca do Gumroad","Gumroad-Bibliothek"],
"Your Cart":["Ton panier","Tu carrito","Seu carrinho","Dein Warenkorb"],
"Order summary":["Récapitulatif","Resumen del pedido","Resumo do pedido","Bestellübersicht"],
"Subtotal":["Sous-total","Subtotal","Subtotal","Zwischensumme"],
"Promo code":["Code promo","Código promocional","Código promocional","Gutscheincode"],
"Enter code":["Entre le code","Introduce el código","Digite o código","Code eingeben"],
"Apply":["Appliquer","Aplicar","Aplicar","Anwenden"],
"Secure payment via Gumroad · instant download after purchase.":["Paiement sécurisé via Gumroad · téléchargement immédiat après l'achat.","Pago seguro con Gumroad · descarga inmediata tras la compra.","Pagamento seguro via Gumroad · download imediato após a compra.","Sichere Zahlung über Gumroad · Sofort-Download nach dem Kauf."],
"Multiple items are combined into one Gumroad payment.":["Plusieurs articles sont regroupés en un seul paiement Gumroad.","Varios artículos se combinan en un solo pago de Gumroad.","Vários itens são combinados em um único pagamento no Gumroad.","Mehrere Artikel werden zu einer Gumroad-Zahlung zusammengefasst."],
"Buy 5, get 5 free — unlocked!":["5 achetés, 5 offerts — débloqué !","Compra 5, llévate 5 gratis — ¡desbloqueado!","Compre 5, leve 5 grátis — desbloqueado!","Kauf 5, bekomm 5 gratis — freigeschaltet!"],
"These plugins ride free in your Gumroad checkout, automatically.":["Ces plugins sont ajoutés gratuitement à ton paiement Gumroad, automatiquement.","Estos plugins van gratis en tu pago de Gumroad, automáticamente.","Esses plugins vão grátis no seu pagamento Gumroad, automaticamente.","Diese Plugins sind automatisch gratis in deinem Gumroad-Checkout."],
"Complete your setup":["Complète ton setup","Completa tu setup","Complete seu setup","Vervollständige dein Setup"],
"Order confirmed":["Commande confirmée","Pedido confirmado","Pedido confirmado","Bestellung bestätigt"],
"Thank you!":["Merci !","¡Gracias!","Obrigado!","Danke!"],
"Loading your downloads…":["Chargement de tes téléchargements…","Cargando tus descargas…","Carregando seus downloads…","Deine Downloads werden geladen…"],
"Bookmark this page:":["Mets cette page en favori :","Guarda esta página:","Salve esta página:","Setz ein Lesezeichen:"],
"Terms & License":["Conditions & licence","Condiciones y licencia","Termos e licença","Bedingungen & Lizenz"],

/* ---------- product pages (shared template) ---------- */
"Instant download · Secure checkout on Gumroad":["Téléchargement immédiat · Paiement sécurisé sur Gumroad","Descarga inmediata · Pago seguro en Gumroad","Download imediato · Pagamento seguro no Gumroad","Sofort-Download · Sichere Zahlung bei Gumroad"],
"Royalty-free sounds":["Sons libres de droits","Sonidos libres de regalías","Sons livres de royalties","Lizenzfreie Sounds"],
"Overview":["Présentation","Descripción","Visão geral","Überblick"],
"← Back to catalog":["← Retour au catalogue","← Volver al catálogo","← Voltar ao catálogo","← Zurück zum Katalog"],
"Share:":["Partager :","Compartir:","Compartilhar:","Teilen:"],
"More free drum kits and sample packs":["Plus de drum kits et sample packs gratuits","Más drum kits y sample packs gratis","Mais drum kits e sample packs grátis","Mehr kostenlose Drum Kits und Sample Packs"],
"Free drum kits and sample packs":["Drum kits et sample packs gratuits","Drum kits y sample packs gratis","Drum kits e sample packs grátis","Kostenlose Drum Kits und Sample Packs"],
"Hear it in action":["Écoute-le en action","Escúchalo en acción","Ouça em ação","Hör es in Aktion"],
"Download for free":["Télécharger gratuitement","Descargar gratis","Baixar grátis","Kostenlos herunterladen"],
"Made by DJBILBOX BEATS":["Créé par DJBILBOX BEATS","Hecho por DJBILBOX BEATS","Feito por DJBILBOX BEATS","Von DJBILBOX BEATS gemacht"],
"Buy now":["Acheter maintenant","Comprar ahora","Comprar agora","Jetzt kaufen"],
"Get it on Gumroad":["L'obtenir sur Gumroad","Consíguelo en Gumroad","Obter no Gumroad","Bei Gumroad holen"],
"Drum machines and generators":["Boîtes à rythmes et générateurs","Cajas de ritmos y generadores","Baterias eletrônicas e geradores","Drum Machines und Generatoren"],
"Key features":["Fonctions clés","Funciones clave","Recursos principais","Hauptfunktionen"],
"Features":["Fonctions","Funciones","Recursos","Funktionen"],
"System requirements":["Configuration requise","Requisitos del sistema","Requisitos do sistema","Systemanforderungen"],
"Technical specs":["Caractéristiques techniques","Especificaciones técnicas","Especificações técnicas","Technische Daten"],
"Synths and instruments":["Synthés et instruments","Sintes e instrumentos","Sintetizadores e instrumentos","Synths und Instrumente"],
"More synths and instruments":["Plus de synthés et d'instruments","Más sintes e instrumentos","Mais sintetizadores e instrumentos","Mehr Synths und Instrumente"],
"In the download":["Dans le téléchargement","En la descarga","No download","Im Download"],
"Royalty-free license":["Licence libre de droits","Licencia libre de regalías","Licença livre de royalties","Lizenzfreie Lizenz"],
"Royalty-free licence":["Licence libre de droits","Licencia libre de regalías","Licença livre de royalties","Lizenzfreie Lizenz"],
"Perfect for":["Idéal pour","Perfecto para","Perfeito para","Perfekt für"],
"Effects":["Effets","Efectos","Efeitos","Effekte"],
"More effects":["Plus d'effets","Más efectos","Mais efeitos","Mehr Effekte"],
"Bundles":["Bundles","Bundles","Bundles","Bundles"],
"More bundles":["Plus de bundles","Más bundles","Mais bundles","Mehr Bundles"],
"Windows installer":["Installateur Windows","Instalador de Windows","Instalador do Windows","Windows-Installer"],
"PDF user manual":["Manuel d'utilisation PDF","Manual de usuario en PDF","Manual do usuário em PDF","PDF-Handbuch"],
"English user manual included":["Manuel en anglais inclus","Manual en inglés incluido","Manual em inglês incluído","Englisches Handbuch enthalten"],
"User manual (FR / EN) and install guide inside each zip":["Manuel (FR / EN) et guide d'installation dans chaque zip","Manual (FR / EN) y guía de instalación en cada zip","Manual (FR / EN) e guia de instalação em cada zip","Handbuch (FR / EN) und Installationsanleitung in jedem Zip"],
"4 GB RAM minimum, 8 GB recommended":["4 Go de RAM minimum, 8 Go recommandés","4 GB de RAM mínimo, 8 GB recomendados","4 GB de RAM no mínimo, 8 GB recomendados","Mind. 4 GB RAM, 8 GB empfohlen"],
"Windows 10 or 11, 64-bit: VST3":["Windows 10 ou 11, 64 bits : VST3","Windows 10 u 11, 64 bits: VST3","Windows 10 ou 11, 64 bits: VST3","Windows 10 oder 11, 64 Bit: VST3"],
"Windows 10/11 64-bit: VST3 + Standalone app":["Windows 10/11 64 bits : VST3 + application autonome","Windows 10/11 64 bits: VST3 + app independiente","Windows 10/11 64 bits: VST3 + app standalone","Windows 10/11 64 Bit: VST3 + Standalone-App"],
"Which DAWs does it work in?":["Dans quels DAW il fonctionne ?","¿En qué DAW funciona?","Em quais DAWs funciona?","In welchen DAWs läuft es?"],
"Does it use samples?":["Est-ce qu'il utilise des samples ?","¿Usa samples?","Ele usa samples?","Nutzt es Samples?"],
"Can I save my own presets?":["Je peux sauvegarder mes propres presets ?","¿Puedo guardar mis propios presets?","Posso salvar meus próprios presets?","Kann ich eigene Presets speichern?"],
"Interface in French or English, resizable from 75 to 200 %.":["Interface en français ou en anglais, redimensionnable de 75 à 200 %.","Interfaz en francés o inglés, redimensionable del 75 al 200 %.","Interface em francês ou inglês, redimensionável de 75 a 200 %.","Oberfläche auf Französisch oder Englisch, skalierbar von 75 bis 200 %."],
"Royalty-free — Everything you make with it is yours.":["Libre de droits — tout ce que tu crées avec est à toi.","Libre de regalías — todo lo que hagas con él es tuyo.","Livre de royalties — tudo o que você cria com ele é seu.","Lizenzfrei — alles, was du damit machst, gehört dir."],
"Use it in private and public, personal and commercial projects: releases, beats for sale, streaming, live shows, film, games, client work. Everything you make is 100 % yours — no royalties, no credit required.":["Utilise-le dans tes projets privés et publics, perso et commerciaux : sorties, instrus à vendre, streaming, concerts, films, jeux, travaux clients. Tout ce que tu crées est 100 % à toi — sans royalties, sans crédit obligatoire.","Úsalo en proyectos privados y públicos, personales y comerciales: lanzamientos, beats a la venta, streaming, conciertos, cine, juegos, trabajos para clientes. Todo lo que hagas es 100 % tuyo — sin regalías ni créditos obligatorios.","Use em projetos privados e públicos, pessoais e comerciais: lançamentos, beats à venda, streaming, shows, filmes, jogos, trabalhos para clientes. Tudo o que você faz é 100 % seu — sem royalties, sem crédito obrigatório.","Nutze es in privaten und öffentlichen, persönlichen und kommerziellen Projekten: Releases, Beats zum Verkauf, Streaming, Live-Shows, Film, Games, Kundenaufträge. Alles, was du machst, gehört zu 100 % dir — keine Tantiemen, kein Credit nötig."],
"Click-free BYPASS footswitch":["Footswitch BYPASS sans clic","Footswitch BYPASS sin clics","Footswitch BYPASS sem cliques","Klickfreier BYPASS-Fußschalter"],
"⚡ Instant Download right after checkout.":["⚡ Téléchargement immédiat juste après le paiement.","⚡ Descarga inmediata justo después del pago.","⚡ Download imediato logo após o pagamento.","⚡ Sofort-Download direkt nach dem Kauf."],
"📦 WHAT YOU GET":["📦 CE QUE TU REÇOIS","📦 LO QUE RECIBES","📦 O QUE VOCÊ RECEBE","📦 WAS DU BEKOMMST"],
"💻 FORMATS":["💻 FORMATS","💻 FORMATOS","💻 FORMATOS","💻 FORMATE"],
"✅ TESTED FOR REAL":["✅ TESTÉ POUR DE VRAI","✅ PROBADO DE VERDAD","✅ TESTADO DE VERDADE","✅ WIRKLICH GETESTET"],
"🔥 36 GENRES IN ONE PLUGIN":["🔥 36 STYLES DANS UN SEUL PLUGIN","🔥 36 GÉNEROS EN UN PLUGIN","🔥 36 GÊNEROS EM UM PLUGIN","🔥 36 GENRES IN EINEM PLUGIN"],
"🇫🇷 EN FRANÇAIS":["🇫🇷 EN FRANÇAIS","🇫🇷 EN FRANCÉS","🇫🇷 EM FRANCÊS","🇫🇷 AUF FRANZÖSISCH"],
"WHAT'S INSIDE?":["QU'Y A-T-IL DEDANS ?","¿QUÉ INCLUYE?","O QUE TEM DENTRO?","WAS IST DRIN?"],
"This bundle contains...":["Ce bundle contient…","Este bundle contiene…","Este bundle contém…","Dieses Bundle enthält…"],
"Qty: 1":["Qté : 1","Cant.: 1","Qtd.: 1","Menge: 1"],
"Save":["Économise","Ahorra","Economize","Spare"],
"Copy code":["Copier le code","Copiar el código","Copiar o código","Code kopieren"],
};

/* ---------- engine ---------- */
const qs = new URLSearchParams(location.search);
let lang = (qs.get('lang')||'').toLowerCase();
const pageLang = (document.documentElement.lang||'en').slice(0,2).toLowerCase();
if (!LANGS.includes(lang)) { try { lang = localStorage.getItem(KEY)||''; } catch(e){} }
if (!LANGS.includes(lang)) {
  const nav = (navigator.language||'en').slice(0,2).toLowerCase();
  lang = pageLang !== 'en' && LANGS.includes(pageLang) ? pageLang : (LANGS.includes(nav) ? nav : 'en');
}
if (qs.get('lang')) { try { localStorage.setItem(KEY, lang); } catch(e){} }

/* static per-language pages: go to the right version */
const parts = location.pathname.split('/').filter(Boolean);
const file = parts[parts.length-1]||'index.html';
const inLangDir = parts.length>1 && LANGS.includes(parts[parts.length-2]);
function staticUrl(l){ return (l==='en'?'/':'/'+l+'/')+file; }
if (STATIC.includes(file)) {
  const cur = inLangDir ? parts[parts.length-2] : 'en';
  if (cur !== lang) { location.replace(staticUrl(lang)); return; }
}

const i = IDX[lang];
const done = new WeakMap();
const norm = s => s.replace(/\s+/g,' ').trim();
function tr(s){ const v = D[norm(s)]; return v ? v[i] : null; }
function doText(n){
  if (done.get(n) === n.nodeValue) return;
  const raw = n.nodeValue, t = tr(raw);
  if (t) { const lead = raw.match(/^\s*/)[0], tail = raw.match(/\s*$/)[0]; n.nodeValue = lead + t + tail; }
  done.set(n, n.nodeValue);
}
function doEl(el){
  for (const a of ['placeholder','aria-label','title']) {
    const v = el.getAttribute && el.getAttribute(a); if (v) { const t = tr(v); if (t) el.setAttribute(a, t); }
  }
}
function walk(root){
  if (!root) return;
  if (root.nodeType === 3) { doText(root); return; }
  if (root.nodeType !== 1 || /^(SCRIPT|STYLE|NOSCRIPT|TEXTAREA|CODE)$/.test(root.tagName) || root.closest('[data-no-i18n]')) return;
  doEl(root);
  const w = document.createTreeWalker(root, NodeFilter.SHOW_TEXT|NodeFilter.SHOW_ELEMENT, {acceptNode(n){
    if (n.nodeType===1) return /^(SCRIPT|STYLE|NOSCRIPT|TEXTAREA|CODE)$/.test(n.tagName) || n.hasAttribute('data-no-i18n') ? NodeFilter.FILTER_REJECT : NodeFilter.FILTER_ACCEPT;
    return NodeFilter.FILTER_ACCEPT; }});
  let n; while ((n = w.nextNode())) { if (n.nodeType===3) doText(n); else doEl(n); }
}

function picker(){
  if (document.querySelector('.hp-lang')) return;
  const sel = document.createElement('select');
  sel.className = 'hp-lang'; sel.setAttribute('data-no-i18n',''); sel.setAttribute('aria-label', lang==='en'?'Language':D.Language[i]);
  sel.innerHTML = LANGS.map(l=>`<option value="${l}"${l===lang?' selected':''}>${l.toUpperCase()} · ${NAMES[l]}</option>`).join('');
  sel.onchange = () => {
    const l = sel.value; try { localStorage.setItem(KEY, l); } catch(e){}
    if (STATIC.includes(file)) { location.href = staticUrl(l); return; }
    const u = new URL(location.href); u.searchParams.delete('lang'); location.href = u.toString().replace(/#$/,'');
  };
  const st = document.createElement('style');
  st.textContent = `.hp-lang{appearance:none;-webkit-appearance:none;background:rgba(255,255,255,.06) url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='10' height='6'%3E%3Cpath d='M0 0l5 6 5-6z' fill='%23aaa'/%3E%3C/svg%3E") no-repeat right 10px center;color:#e9eeeb;border:1px solid rgba(255,255,255,.16);border-radius:999px;padding:7px 28px 7px 12px;font:600 12px/1 Inter,system-ui,sans-serif;cursor:pointer;max-width:132px}
.hp-lang option{background:#111;color:#eee}
.hp-lang.float{position:fixed;left:14px;bottom:14px;z-index:9999;background-color:rgba(10,10,10,.85);backdrop-filter:blur(8px)}
@media(max-width:720px){.top-actions .hp-lang{max-width:64px;padding-right:22px;font-size:11px}}`;
  document.head.appendChild(st);
  const host = document.querySelector('.top-actions, .pp-top-in .pp-store, .pp-top-in');
  if (host && host.classList.contains('top-actions')) host.insertBefore(sel, host.firstChild);
  else if (host && host.classList.contains('pp-store')) host.parentNode.insertBefore(sel, host);
  else if (host) host.appendChild(sel);
  else { sel.classList.add('float'); document.body.appendChild(sel); }
  if (window.innerWidth < 720) [...sel.options].forEach(o => o.textContent = o.value.toUpperCase());
}

function start(){
  if (lang !== 'en') {
    document.documentElement.lang = lang;
    const t = tr(document.title); if (t) document.title = t;
    walk(document.body);
    new MutationObserver(ms => { for (const m of ms) {
      if (m.type === 'characterData') doText(m.target);
      else m.addedNodes.forEach(walk);
    }}).observe(document.body, {childList:true, subtree:true, characterData:true});
  }
  picker();
  /* the site header is injected by site.js: retry until it exists */
  let tries = 0; const iv = setInterval(()=>{ const s=document.querySelector('.hp-lang.float'); const h=document.querySelector('.top-actions');
    if (s && h) { s.classList.remove('float'); h.insertBefore(s, h.firstChild); }
    if (++tries > 20 || !s) clearInterval(iv); }, 150);
}
window.HP_LANG = lang;
if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();
})();
