# הנחיות עבודה בפרויקט

## Demo/executable policy

Before adding a new standalone demo `main()` + CMake `add_executable` target for a new step,
first check whether an existing demo executable has already been fully superseded: does a
later/more-complete demo already prove everything the older one proves, plus more (same
reasoning as the S4.5 cleanup step)? If so, REPLACE that file's content and repurpose its
existing CMake target for the new step's demo, instead of creating a new file and a new target.

Only create a genuinely new demo file/target when no existing one is a reasonable candidate for
reuse (e.g. it tests a still-unique code path, or it's the current in-progress step and not yet
verified).

This keeps the number of demo executables from growing unbounded as steps accumulate, so
CMakeLists.txt and the project structure stay navigable.

## עברית ו-RTL בתשובות בצ'אט

כשאני כותב תשובות בעברית (טקסט רגיל, לא קוד), אני פועל לפי הכללים הבאים כדי שהקריאה תהיה נוחה בטרמינל RTL:

- מונחים באנגלית, שמות קבצים, פקודות וקוד בתוך משפט עברי - תמיד בתוך `inline code` (backticks), כדי שלא ישברו את כיוון הקריאה.
- לא מערבבים עברית ואנגלית באותו משפט בלי הפרדה ברורה של backticks/סוגריים.
- העדפה לרשימות קצרות (bullets) על פני פסקאות ארוכות - קל יותר לעין לעקוב מימין לשמאל.
- כותרות ותוויות קצרות ותכל'ס, בלי מבנה מקונן מדי.
- שמירה על פיסוק עברי תקין (מקף לא רווח כפול, גרשיים " ולא ציטוט אנגלי כפול כשמדובר בטקסט עברי).

## עמודי Artifact / HTML בעברית

כשאני בונה Artifact (HTML) שהתוכן שלו בעברית, אני משתמש בסקיל `hebrew-rtl-artifact` שמגדיר את התבנית הנכונה (dir="rtl", פונט, יישור, מראה של layout).
