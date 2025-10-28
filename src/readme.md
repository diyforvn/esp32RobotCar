Quy trình tuning từng bước 

Quan sát:

Nếu robot không rẽ đủ vào line: tăng Kp +10% (ví dụ 33 → 36).

Nếu robot rung/nhấp (oscillate): giảm Kp 10% hoặc tăng Kd nhẹ (tăng Kd +1).

Tuning Kp:

Tăng Kp cho tới khi robot bắt đầu dao động nhẹ, note giá trị Kp_crit.

Thiết lập Kp = 0.6 * Kp_crit (mẹo) — hoặc chọn giá trị trước dao động mà bạn thấy "về line nhanh nhưng ổn".

Tuning Kd:

Bắt đầu với Kd = Kp * 0.25 (ước lượng), sau đó tăng để giảm dao động. Với Kp~30 → Kd bắt đầu ~7–8 (mình chọn 8).

Ki:

Giữ Ki=0 ban đầu. Nếu robot có offset cố định (luôn lệch bên), bật Ki nhỏ 0.1 rồi tăng chậm (tối đa 1.0). Đừng để Ki quá lớn — gây tích phân vượt.

Thử với tốc độ khác:

Khi baseSpeed tăng, Kp nên tăng tương ứng (khoảng tỉ lệ), Kd cũng tăng một chút.

Fine tuning lostCount behavior:

Nếu robot mất line nhiều lần: tăng MAX giả lập (min cap) hoặc tăng lostCount slope để robot quay mạnh hơn khi mất lâu.

✅ 5) Các mẹo thực tế

Giữ baseSpeed thấp lúc tune (100–120). Khi hoàn tất, tăng dần.

Nếu thấy I tích tụ lớn → giảm Ki hoặc giảm INTEGRAL_LIMIT.

Nếu robot quay quá chậm tìm line → tăng MAX_ERR trong Task_LineReader (giá trị giả lập) từ 2 → 3.

Dùng Serial.printf debug để quan sát ERR/CORR/I/D như code trên



Robot quay chậm, không vào tâm → tăng Kp +10% (ví dụ 30 → 33).

Robot nhấp/lắc → giảm Kp 10% hoặc tăng Kd +2.

Robot quay quá mạnh, overshoot → tăng Kd hoặc giảm max corr (MAX_CORR).
