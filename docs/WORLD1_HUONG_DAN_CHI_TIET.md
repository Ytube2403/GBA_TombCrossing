# World 1 - Kich ban va huong dan chi tiet 20 man

Cap nhat: 2026-06-12

## Ket luan kiem tra

- 20/20 man co spawn, exit va duong di hop le.
- Tu man 05 tro di, do kho tang theo hook ro hon: gap jump, landing bait, chase pressure va recovery pocket ngan.
- Man 14, 18, 20 la cac man can human playtest uu tien de danh gia do hieu va do cong bang.
- World 1 chi dung tap palette an toan: Vanish Floor, Pop Spike, Sand Sinkhole, Boulder, Falling Block va gai co ban.

Toa do trong tai lieu la tile Tiled zero-based `(x,y)`.

## Quy tac trap

| Trap | Cach hoat dong | Cach xu ly |
|---|---|---|
| Vanish Floor | San nut bien mat sau so frame da dat | Chay lien tuc, nhay truoc khi strip bien mat |
| Pop Spike | Gai bat len khi player vao ban kinh kich hoat | Bait, cho rut, hoac vuot qua tren khong |
| Sand Sinkhole | Dung lien tuc se bi nhan va chet | Khong dung lai; hop hoac chay qua |
| Falling Block | Rung canh bao roi roi thang xuong | Leave column ngay khi bi kich hoat |
| Boulder | Lan tu truoc mat hoac phia sau sau trigger | Nhay qua, bao toc do, hoac dung do cao |

## Man 01 - Cracked Promise

- **Vai tro:** Teach Vanish Floor.
- **Spawn/Exit:** `(2,15)` -> `(27,15)`.
- **Trap:** Vanish `(14,16)`, delay 18.
- **Punishment:** Spike bed `y=17, x=13..15`.

**Cach qua:** Chay sang phai, nhay qua strip nut va doi dat sau spikes.

## Man 02 - Teeth in the Sand

- **Vai tro:** Teach Pop Spike.
- **Trap:** Pop Spike `(14,16)`, ban kinh 3 tile, active 55 frame.

**Cach qua:** Kich hoat gai tu ben ngoai tile, doi rut, roi qua; route nhanh la nhay qua trong luc gai dang extend.

## Man 03 - Four Mouths

- **Vai tro:** Teach Sand Sinkhole rhythm.
- **Trap:** Sinkhole `(10,16)`, `(12,16)`, `(15,16)`, `(17,16)`.

**Cach qua:** Giua nhịp chay hoac short-hop; khong dung tren xoay cat.

## Man 04 - Rolling Welcome

- **Vai tro:** Teach Boulder jump timing.
- **Trap:** Boulder `(13,15)`, toc do 2.0, spawn tu phai.

**Cach qua:** Qua trigger, doc Boulder tu phai va nhay qua no.

## Man 05 - Stonefall Gap

- **Vai tro:** Falling Block gap commitment.
- **Trap:** Falling Block `(11,10)`, `(13,10)`.
- **Dia hinh:** Ho gap co spike bed ben duoi.

**Cach qua:** Kich hoat block, roi khoi cot, sau do commit sang gap.

## Man 06 - No Time to Stand

- **Vai tro:** Vanish runway into Pop Spike bait.
- **Trap:** Vanish `(8,16)`, Pop Spike `(17,16)`.

**Cach qua:** Dung strip Vanish lam runway, bay qua gap, roi bait Pop Spike o landing.

## Man 07 - Sand and Stone

- **Vai tro:** Sand rhythm under Boulder pressure.
- **Trap:** Sinkhole `(9,16)`, `(11,16)`, `(13,16)`, Boulder `(16,15)`.

**Cach qua:** Qua cat bang nhip chay, sau do doc va nhay Boulder ngay.

## Man 08 - Falling Rhythm

- **Vai tro:** Falling gaps and landing Pop Spike.
- **Trap:** Falling `(8,8)`, `(10,8)`, `(20,8)`, Pop Spike `(14,15)`.

**Cach qua:** Qua khoi roi dau, roi bait gai o landing, sau do xu ly khoi roi cuoi.

## Man 09 - Two Cracked Roads

- **Vai tro:** Chase pressure va hai strip Vanish.
- **Trap:** Boulder `(3,15)`, Vanish `(6,16)`, `(15,16)`.

**Cach qua:** Chay theo Boulder, commit qua strip dau, recover ngan, roi qua strip hai.

## Man 10 - Order Break

- **Vai tro:** Reorder Sand va Falling.
- **Trap:** Sinkhole va Falling Block xen ke.

**Cach qua:** Giai quyet nua trai, lanh lai o pocket giua, sau do lap lai o nua phai.

## Man 11 - Three-Way Chase

- **Vai tro:** Chase remix voi cac quyet dinh tach biet.
- **Trap:** Boulder, hai Pop Spike, hai strip Vanish.

**Cach qua:** Khong dung lau; lay nhip qua tung beat rieng.

## Man 12 - Short Pockets

- **Vai tro:** Compress Falling, Sand va Pop.
- **Trap:** Falling Block, Sinkhole, Pop Spike theo tung doan ngan.

**Cach qua:** Xem nhu ba doan read-act-land ngan, co diem hoi phuc ro rang.

## Man 13 - Pursuit Relay

- **Vai tro:** Alternating terrain chase relay.
- **Trap:** Boulder, Vanish, Sinkhole.

**Cach qua:** Giu momentum; moi beat chi doi mot thoi diem commit.

## Man 14 - Rising Under Pressure

- **Vai tro:** Falling-step setup under pressure.
- **Trap:** Falling Block + Pop Spike + Boulder.

**Cach qua:** Block roi tao landmark, Pop Spike tao point commit, Boulder ep di len.

## Man 15 - Broken Terraces

- **Spawn/Exit:** `(5,12)` -> `(27,14)`.
- **Trap:** Pop Spike `(6,13)`, Sinkhole `(13,12)`, `(15,12)`, Pop Spike `(20,11)`, Vanish `(8,16)`, `(21,16)`.

**Cach qua:** Bat dau tren terrace trai, vuot qua hai Sinkhole giua, xu ly Pop Spike tren cao va xuong exit ben phai.

## Man 16 - Stairway Under Pursuit

- **Spawn/Exit:** `(2,15)` -> `(27,5)`.
- **Trap:** Boulder `(3,15)`, Vanish `(7,16)`, Pop Spike `(6,13)`, `(12,10)`, Sinkhole `(12,16)`, `(14,16)`, `(19,8)`.

**Cach qua:** Chay trong Boulder pressure, leo qua tung bac 24 px va dung nhap nhay tai cac diem commit.

## Man 17 - Three Betrayal Beats

- **Trap:** Ba beat Vanish + Falling Block + Pop Spike.

**Cach qua:** Lap nhịp ba lan, moi lan doi spacing hoac delay, khong auto-pilot.

## Man 18 - Build Your First Step

- **Spawn/Exit:** `(2,15)` -> `(27,4)`.
- **Trap:** Boulder `(3,15)`, Falling `(6,12)`, `(7,12)`, Sinkhole `(10,16)`, Pop Spike `(14,10)`, Falling `(17,8)`, Sinkhole `(20,7)`, Pop Spike `(26,5)`.

**Cach qua:** Dung Falling Blocks lam bac buoc, roi leo tiep qua phan pressure ben tren.

## Man 19 - Recovery Recap

- **Trap:** Vanish, Falling Block, Pop Spike, Sinkhole, Boulder theo nhp ngan.

**Cach qua:** Khong co luat moi; chi la tong hop co khoang hoi phuc ngan giua cac beat.

## Man 20 - Thử thách của Ra

- **Spawn/Exit:** `(2,15)` -> `(6,4)`.
- **Trap:** Boulder `(2,15)`, Vanish 3 strip, 3 Pop Spike, 2 Sinkhole, 4 Falling Block.

**Cach qua phase mat dat:** Giu grace dau, qua 3 phase ground, nhay qua Sinkhole va len platform phai.

**Cach qua phase leo bac:** Leo trai qua 3 bac 24 px va 1 bac cuoi 16 px den exit.

## Gate human playtest

- So lan clear dau tien cua tung man.
- Learned clear time duoi 30 giay.
- Ty le giai thich dung nguyen nhan chet sau lan gap dau.
- Muc do hieu Falling Block o man 14 va 18.
- Phase death cua man 20.
