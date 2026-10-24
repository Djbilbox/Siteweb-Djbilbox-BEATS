/* ============================================================
   PROMO −30 % — vrai code Gumroad HUMPIRE30
   ------------------------------------------------------------
   Créé le 2026-10-10 sur Gumroad (Checkout > Discounts) : −30 %,
   valable jusqu'au 31 octobre 2026 23:59 (heure de Paris), sur
   TOUS les produits payants (code étendu le 2026-10-24). Le code est collé automatiquement aux liens
   d'achat (/l/<slug>/HUMPIRE30), donc le client paie exactement
   le prix affiché.

   ELIGIBLE doit rester IDENTIQUE à la liste des produits du code
   côté Gumroad. Un produit affiché en promo ici mais absent du
   code serait facturé plein tarif (et inversement). Vérifier avec
   un lien /l/<slug>/HUMPIRE30 : `discount_code.valid` doit valoir
   true.

   Exclus volontairement (bil, 2026-10-10) : HUMPIRE VST SUITE et ses 7
   plugins (BLOCK GORILLA, CASBAH CHORDS, TAXI ALGER, CAMEL PILOT, ALIEN 808,
   CHOCOLATINE, LA BAGUETTE), JUKEBOX NEW YORK, le bundle 25 plugins (il
   contient la Suite) et EQ-PRO SPIDER. Tout le reste est à −30 %, y compris
   les prix relevés le jour même : le client doit pouvoir vérifier que le
   prix de référence (barré) est bien le plus bas des 30 jours précédents.

   Après la date de fin, tout redevient normal sans rien toucher.
   ============================================================ */
window.Promo = (function () {
  const CODE = 'HUMPIRE30';
  const PERCENT = 30;
  const UNTIL = Date.parse('2026-10-31T23:59:00+01:00');

  /* slugs d'achat utilisés par le site + permalinks courts Gumroad */
  const ELIGIBLE = new Set(["aajff","ajpebi","alien-808","aurora-verb","auxhyu","awwbw","block-gorilla","bsscfh","camel-pedal","camel-pedals","camel-pilot","canyon-echo","capesm","casbah-chords","cbbot","chdoa","chocolatine","coral-chorus","dfosrq","dipvtj","djbilbox-pro-bundle","douhjk","dzranx","fhpnge","fimvw","gjebft","hfmjow","hntzne","hqlss","humpire-vst-suite","imqzs","iqwrwh","jdfemx","jdrqvz","jet-flanger","jukebox-new-york","jvdta","jxgob","krwhfy","la-baguette","lava-fuzz","lqcun","mastering","mjnau","nbdpba","neon-phaser","nxkwfd","ocpoej","ohiyxu","oriental-instrument-djbilbox-beats","owqnh","oyomwb","ozpudm","pharaoh-comp","qcuvdq","qkjjl","qmsaon","quwjty","sgfzgi","sirmb","station-synth-legendary-bundle","storm-rider","surf-tremolo","svdue","szmhtb","taxi-alger","tfhvc","thuglife","uetjad","uvxck","vptjlg","vslvad","wbmbpi","wgbeza","wlkzna","wrvnxn","xaziro","xwrzb","xxbxebh","ykdzli","yrkzl","yvlyecx","zfhkh","zfkdea","zqyvvu","zufkrv","zzcpzf"]);

  const num = v => parseFloat(String(v).replace(',', '.').replace(/[^0-9.]/g, '')) || 0;
  const active = () => Date.now() < UNTIL;
  const ok = buy => active() && ELIGIBLE.has(buy);
  /* arrondi au centime, comme Gumroad (prix en cents × 70 %) */
  const sale = list => Math.round(num(list) * (100 - PERCENT)) / 100;
  const fmt = n => '$' + (Number.isInteger(n) ? n : n.toFixed(2));

  /* prix écrits en dur dans les pages statiques : <span data-djb-sale="slug" data-list="99">$99</span> */
  function paint() {
    document.querySelectorAll('[data-djb-sale]').forEach(el => {
      const slug = el.getAttribute('data-djb-sale'), list = el.getAttribute('data-list');
      if (ok(slug)) el.innerHTML = '<s style="opacity:.55;font-weight:500">' + fmt(num(list)) + '</s> ' + fmt(sale(list));
    });
  }
  if (typeof document !== 'undefined') {
    if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', paint); else paint();
  }

  return {
    CODE, PERCENT, UNTIL, active, ok, num, sale, fmt, paint,
    codeFor: buy => (ok(buy) ? CODE : ''),
    /* prix réellement payé pour ce produit (nombre) */
    price: (buy, list) => (ok(buy) ? sale(list) : num(list))
  };
})();
