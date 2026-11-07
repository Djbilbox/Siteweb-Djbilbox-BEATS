/* ============================================================
   PROMO CODE — one store-wide code, shown the same way everywhere.
   The code must exist in Gumroad (Products → Discounts) with the
   same percentage, otherwise checkout refuses it: keep `active`
   false until it does. Buy buttons add the code to the Gumroad URL
   (/l/<id>/<CODE>), so it is also applied without typing it.
   ============================================================ */
window.HUMPIRE_PROMO = {
  active: false,
  code: 'HUMPIRE30',
  percent: 30,
  title: 'Launch sale',
  ends: '2026-12-01T00:00:00+01:00'
};
