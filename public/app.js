const searchForm = document.querySelector("#search-form");
const checkInInput = document.querySelector("#check-in");
const checkOutInput = document.querySelector("#check-out");
const roomList = document.querySelector("#room-list");
const roomMessage = document.querySelector("#room-message");
const bookingDialog = document.querySelector("#booking-dialog");
const bookingForm = document.querySelector("#booking-form");
const bookingMessage = document.querySelector("#booking-message");
const bookingList = document.querySelector("#booking-list");
const toast = document.querySelector("#toast");
const themeToggle = document.querySelector("#theme-toggle");
const themeIcon = themeToggle.querySelector(".theme-icon");
const themeLabel = themeToggle.querySelector(".theme-label");
const themeStorageKey = "hotel-haven-theme";
const slideshow = document.querySelector("#room-slideshow");
const slideshowImage = document.querySelector("#slideshow-image");
const slideshowTitle = document.querySelector("#slideshow-title");
const slideshowCount = document.querySelector("#slideshow-count");
const slideshowAnnouncement = document.querySelector("#slideshow-announcement");
const slideshowDots = Array.from(document.querySelectorAll(".slide-dot"));
const slideshowToggle = document.querySelector("#slideshow-toggle");
const roomSlides = [
  { name: "Classic King", image: "/images/standard-room.jpg?v=1" },
  { name: "Deluxe Garden", image: "/images/deluxe-room.jpg?v=1" },
  { name: "The Corner Suite", image: "/images/suite-room.jpg?v=1" }
];
const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)");
let activeSlide = 0;
let slideshowTimer;
let slideshowPaused = false;

function applyTheme(theme) {
  const isDark = theme === "dark";
  document.documentElement.dataset.theme = isDark ? "dark" : "light";
  themeToggle.setAttribute("aria-pressed", String(isDark));
  themeToggle.setAttribute("aria-label", `Switch to ${isDark ? "light" : "dark"} mode`);
  themeIcon.textContent = isDark ? "☀" : "☾";
  themeLabel.textContent = isDark ? "Light mode" : "Dark mode";
  document.querySelector('meta[name="theme-color"]').content = isDark ? "#18251f" : "#fffefa";
}

function loadTheme() {
  try {
    const savedTheme = window.localStorage.getItem(themeStorageKey);
    return savedTheme === "dark" ? "dark" : "light";
  } catch (error) {
    console.warn("Could not load the saved theme preference.", error);
    return "light";
  }
}

applyTheme(loadTheme());

themeToggle.addEventListener("click", () => {
  const nextTheme = document.documentElement.dataset.theme === "dark" ? "light" : "dark";
  applyTheme(nextTheme);
  try {
    window.localStorage.setItem(themeStorageKey, nextTheme);
  } catch (error) {
    console.warn("Could not save the theme preference.", error);
  }
});

function showSlide(index, announce = false) {
  activeSlide = (index + roomSlides.length) % roomSlides.length;
  const slide = roomSlides[activeSlide];
  slideshowImage.classList.add("is-changing");
  window.setTimeout(() => {
    slideshowImage.src = slide.image;
    slideshowImage.alt = `${slide.name} hotel room`;
    slideshowTitle.textContent = slide.name.toUpperCase();
    slideshowCount.textContent = `${String(activeSlide + 1).padStart(2, "0")} / ${String(roomSlides.length).padStart(2, "0")}`;
    slideshowDots.forEach((dot, dotIndex) => {
      dot.setAttribute("aria-pressed", String(dotIndex === activeSlide));
    });
    if (announce) {
      slideshowAnnouncement.textContent =
        `Showing ${slide.name}, slide ${activeSlide + 1} of ${roomSlides.length}.`;
    }
    slideshowImage.classList.remove("is-changing");
  }, 160);
}

function startSlideshow(resumedByUser = false) {
  window.clearInterval(slideshowTimer);
  if (slideshowPaused || reducedMotion.matches ||
      (!resumedByUser && (slideshow.matches(":hover") || slideshow.contains(document.activeElement)))) {
    return;
  }
  slideshowTimer = window.setInterval(() => showSlide(activeSlide + 1, false), 5000);
}

slideshowToggle.addEventListener("click", () => {
  slideshowPaused = !slideshowPaused;
  slideshowToggle.setAttribute("aria-pressed", String(slideshowPaused));
  slideshowToggle.setAttribute("aria-label", slideshowPaused ? "Resume slideshow" : "Pause slideshow");
  slideshowToggle.querySelector("span").textContent = slideshowPaused ? "▶" : "Ⅱ";
  startSlideshow(!slideshowPaused);
});

document.querySelector("#slide-previous").addEventListener("click", () => {
  showSlide(activeSlide - 1, true);
  startSlideshow();
});

document.querySelector("#slide-next").addEventListener("click", () => {
  showSlide(activeSlide + 1, true);
  startSlideshow();
});

document.querySelector("#slideshow-dots").addEventListener("click", (event) => {
  const dot = event.target.closest(".slide-dot");
  if (!dot) return;
  showSlide(slideshowDots.indexOf(dot), true);
  startSlideshow();
});

slideshow.addEventListener("mouseenter", () => window.clearInterval(slideshowTimer));
slideshow.addEventListener("mouseleave", startSlideshow);
slideshow.addEventListener("focusin", () => window.clearInterval(slideshowTimer));
slideshow.addEventListener("focusout", (event) => {
  if (!slideshow.contains(event.relatedTarget)) startSlideshow();
});
const handleMotionPreferenceChange = () => startSlideshow();
if (reducedMotion.addEventListener) {
  reducedMotion.addEventListener("change", handleMotionPreferenceChange);
} else {
  reducedMotion.addListener(handleMotionPreferenceChange);
}
startSlideshow();
document.addEventListener("visibilitychange", () => {
  if (document.hidden) {
    window.clearInterval(slideshowTimer);
  } else {
    startSlideshow();
  }
});

const currency = new Intl.NumberFormat("en-IN", {
  style: "currency",
  currency: "INR",
  maximumFractionDigits: 0
});

function localDate(date) {
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, "0");
  const day = String(date.getDate()).padStart(2, "0");
  return `${year}-${month}-${day}`;
}

function escapeHtml(value) {
  return String(value).replace(/[&<>"']/g, (character) => ({
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': "&quot;",
    "'": "&#39;"
  })[character]);
}

function showToast(message) {
  toast.textContent = message;
  toast.classList.add("visible");
  window.setTimeout(() => toast.classList.remove("visible"), 3800);
}

async function request(url, options = {}) {
  const response = await fetch(url, options);
  const payload = await response.json();
  if (!response.ok) {
    throw new Error(payload.error || "Something went wrong. Please try again.");
  }
  return payload;
}

function currentDates() {
  return { checkIn: checkInInput.value, checkOut: checkOutInput.value };
}

function roomCard(room) {
  const theme = room.category.toLowerCase();
  const image = theme === "standard" ? "standard-room.jpg"
    : theme === "deluxe" ? "deluxe-room.jpg" : "suite-room.jpg";
  const amenities = room.amenities.map((amenity) => `<span>${escapeHtml(amenity)}</span>`).join("");
  return `
    <article class="room-card">
      <div class="room-image ${escapeHtml(theme)}">
        <img class="room-photo" src="/images/${image}?v=1" alt="${escapeHtml(room.name)} hotel room" loading="lazy" decoding="async">
        <span class="room-number">ROOM ${room.id}</span>
      </div>
      <div class="room-content">
        <div class="room-meta"><span>${escapeHtml(room.category)} · UP TO ${room.capacity} GUESTS</span><span>NO. ${room.id}</span></div>
        <h3>${escapeHtml(room.name)}</h3>
        <p>${escapeHtml(room.description)}</p>
        <div class="amenities">${amenities}</div>
        <div class="room-bottom">
          <span class="room-price">${currency.format(room.nightlyRate)} <small>/ NIGHT</small></span>
          <button class="button button-dark" type="button" data-room-id="${room.id}" data-room-name="${escapeHtml(room.name)}" data-rate="${room.nightlyRate}" data-capacity="${room.capacity}">Choose room <span aria-hidden="true">↗</span></button>
        </div>
      </div>
    </article>`;
}

async function loadRooms() {
  roomMessage.textContent = "";
  roomList.replaceChildren();
  const { checkIn, checkOut } = currentDates();
  if (!checkIn || !checkOut) {
    roomMessage.textContent = "Choose your check-in and check-out dates to see available rooms.";
    return;
  }
  roomMessage.textContent = "Finding your room...";
  try {
    const params = new URLSearchParams({ checkIn, checkOut });
    const result = await request(`/api/rooms?${params.toString()}`);
    roomMessage.textContent = "";
    if (result.rooms.length === 0) {
      roomList.innerHTML = '<div class="empty-state">No rooms are available for those dates.<br>Try a different stay and we’ll look again.</div>';
      return;
    }
    roomList.innerHTML = result.rooms.map(roomCard).join("");
  } catch (error) {
    roomMessage.textContent = error.message;
  }
}

function openBooking(roomId, roomName, rate, capacity) {
  const dates = currentDates();
  const firstNight = new Date(`${dates.checkIn}T00:00:00`);
  const lastNight = new Date(`${dates.checkOut}T00:00:00`);
  const nights = Math.round((lastNight - firstNight) / 86400000);
  document.querySelector("#selected-room").textContent = `${roomName} · Room ${roomId}`;
  document.querySelector("#stay-summary").textContent =
    `${dates.checkIn} → ${dates.checkOut} · ${nights} ${nights === 1 ? "night" : "nights"} · ${currency.format(rate * nights)} total`;
  document.querySelector("#booking-room-id").value = roomId;
  document.querySelector("#booking-check-in").value = dates.checkIn;
  document.querySelector("#booking-check-out").value = dates.checkOut;
  document.querySelector("#dialog-message").textContent = "";
  bookingForm.reset();
  document.querySelector("#booking-room-id").value = roomId;
  document.querySelector("#booking-check-in").value = dates.checkIn;
  document.querySelector("#booking-check-out").value = dates.checkOut;
  document.querySelector("#guest-count").max = capacity;
  document.querySelector("#guest-count").value = "1";
  bookingDialog.showModal();
}

async function loadBookings(email) {
  bookingMessage.textContent = "Looking up reservations...";
  bookingList.replaceChildren();
  try {
    const params = new URLSearchParams({ email });
    const result = await request(`/api/bookings?${params.toString()}`);
    bookingMessage.textContent = result.bookings.length
      ? `Found ${result.bookings.length} ${result.bookings.length === 1 ? "reservation" : "reservations"}.`
      : "No reservations found for that email address.";
    bookingList.innerHTML = result.bookings.map((booking) => `
      <article class="booking-card">
        <div class="booking-card-top">
          <h3>${escapeHtml(booking.roomName)}</h3>
          <span class="booking-id">${escapeHtml(booking.id)}</span>
        </div>
        <p class="booking-details">${escapeHtml(booking.checkIn)} → ${escapeHtml(booking.checkOut)}<br>
          ${booking.guestCount} ${booking.guestCount === 1 ? "guest" : "guests"} · Guest: ${escapeHtml(booking.guestName)} · Room ${booking.roomId}</p>
        <div class="booking-card-foot">
          <span class="booking-total">${currency.format(booking.totalPrice)}</span>
          <span class="status-pill ${booking.status === "Cancelled" ? "cancelled" : ""}">${escapeHtml(booking.status)}</span>
          ${booking.status === "Confirmed"
            ? `<button class="cancel-button" type="button" data-booking-id="${escapeHtml(booking.id)}">Cancel stay</button>`
            : ""}
        </div>
      </article>`).join("");
  } catch (error) {
    bookingMessage.textContent = error.message;
  }
}

const today = new Date();
const tomorrow = new Date(today);
tomorrow.setDate(today.getDate() + 1);
const dayAfter = new Date(today);
dayAfter.setDate(today.getDate() + 2);
checkInInput.min = localDate(today);
checkOutInput.min = localDate(tomorrow);
checkInInput.value = localDate(tomorrow);
checkOutInput.value = localDate(dayAfter);

checkInInput.addEventListener("change", () => {
  const nextDay = new Date(`${checkInInput.value}T00:00:00`);
  nextDay.setDate(nextDay.getDate() + 1);
  checkOutInput.min = localDate(nextDay);
  if (checkOutInput.value <= checkInInput.value) {
    checkOutInput.value = localDate(nextDay);
  }
});

searchForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  if (checkOutInput.value <= checkInInput.value) {
    roomMessage.textContent = "Check-out must be after check-in.";
    return;
  }
  await loadRooms();
});

roomList.addEventListener("click", (event) => {
  const button = event.target.closest("[data-room-id]");
  if (button) {
    openBooking(button.dataset.roomId, button.dataset.roomName, Number(button.dataset.rate), Number(button.dataset.capacity));
  }
});

bookingForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  const button = bookingForm.querySelector("button[type=submit]");
  button.disabled = true;
  document.querySelector("#dialog-message").textContent = "Confirming your reservation...";
  try {
    const payload = new URLSearchParams(new FormData(bookingForm));
    const result = await request("/api/bookings", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: payload
    });
    bookingDialog.close();
    showToast(`You're booked! Your reference is ${result.booking.id}.`);
    document.querySelector("#lookup-email").value = result.booking.email;
    await loadBookings(result.booking.email);
    await loadRooms();
    document.querySelector("#manage").scrollIntoView({ behavior: "smooth", block: "center" });
  } catch (error) {
    document.querySelector("#dialog-message").textContent = error.message;
  } finally {
    button.disabled = false;
  }
});

document.querySelector(".dialog-close").addEventListener("click", () => bookingDialog.close());
bookingDialog.addEventListener("click", (event) => {
  if (event.target === bookingDialog) bookingDialog.close();
});

document.querySelector("#lookup-form").addEventListener("submit", async (event) => {
  event.preventDefault();
  await loadBookings(document.querySelector("#lookup-email").value.trim());
});

bookingList.addEventListener("click", async (event) => {
  const button = event.target.closest("[data-booking-id]");
  if (!button) return;
  if (!window.confirm(`Cancel reservation ${button.dataset.bookingId}?`)) return;
  button.disabled = true;
  try {
    const payload = new URLSearchParams({
      bookingId: button.dataset.bookingId,
      email: document.querySelector("#lookup-email").value.trim()
    });
    await request("/api/bookings/cancel", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: payload
    });
    showToast("Your reservation has been cancelled.");
    await loadBookings(document.querySelector("#lookup-email").value.trim());
    await loadRooms();
  } catch (error) {
    bookingMessage.textContent = error.message;
    button.disabled = false;
  }
});

loadRooms();
