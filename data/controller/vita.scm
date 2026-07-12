;; data/controller/vita.scm
;; PS Vita controller configuration for Pingus
;;
;; SDL2-vita button mapping (SDL joystick device 0):
;;
;; Button ID | PSV Button
;; ----------+------------
;;     0     | Triangle
;;     1     | Circle
;;     2     | Cross
;;     3     | Square
;;     4     | L Trigger
;;     5     | R Trigger
;;     6     | Down
;;     7     | Left
;;     8     | Up
;;     9     | Right
;;    10     | Select
;;    11     | Start
;;
;; Axis mapping:
;; Axis 0 = Left Stick X
;; Axis 1 = Left Stick Y
;; Axis 2 = Right Stick X
;; Axis 3 = Right Stick Y
;;
;; The left analog stick moves the virtual mouse cursor (via sdl_driver.cpp).
;; Cross = primary click, Circle = secondary click / back.
;; D-Pad scrolls the camera.

(pingus-controller
  ;; No hardware keyboard on Vita, but keep it for debugging via USB
  (standard-keyboard
    (sdl:keyboard))

  ;; Pointer: virtual mouse driven by left analog stick in sdl_driver.cpp
  (standard-pointer
    (sdl:mouse-pointer))

  ;; Camera scrolling: D-Pad buttons (hat) + right analog stick
  (standard-scroller
    (core:button-scroller
      (up    (sdl:joystick-button (device 0) (button 8)))
      (down  (sdl:joystick-button (device 0) (button 6)))
      (left  (sdl:joystick-button (device 0) (button 7)))
      (right (sdl:joystick-button (device 0) (button 9))))
    (core:axis-scroller
      (x-axis (sdl:joystick-axis (device 0) (axis 2)))
      (y-axis (sdl:joystick-axis (device 0) (axis 3)))))

  ;; Primary action: Cross
  (primary-button
    (sdl:joystick-button (device 0) (button 2)))

  ;; Secondary action: Circle
  (secondary-button
    (sdl:joystick-button (device 0) (button 1)))

  ;; Pause: Start
  (pause-button
    (sdl:joystick-button (device 0) (button 11)))

  ;; Fast forward: Select
  (fast-forward-button
    (sdl:joystick-button (device 0) (button 10)))

  ;; Single step: not easily mapped, use L+R combo via square
  (single-step-button
    (sdl:joystick-button (device 0) (button 3)))

  ;; Armageddon: Triangle
  (armageddon-button
    (sdl:joystick-button (device 0) (button 0)))

  ;; Escape / Back to menu: Select
  (escape-button
    (sdl:joystick-button (device 0) (button 10)))

  ;; Cycle available actions upward: L Trigger
  (action-up-button
    (sdl:joystick-button (device 0) (button 4)))

  ;; Cycle available actions downward: R Trigger
  (action-down-button
    (sdl:joystick-button (device 0) (button 5)))

  ;; Direct action slots are not individually bound on Vita
  ;; (only 12 physical buttons; use action-up/down to cycle)
  )

;; EOF ;;
