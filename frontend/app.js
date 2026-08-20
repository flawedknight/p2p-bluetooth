const MAX_TTL = 127;
const HEADER_BYTES = 5;
const encoder = new TextEncoder();
const decoder = new TextDecoder();
const seenIds = new Set();

const elements = {
  form: document.querySelector('#sosForm'),
  message: document.querySelector('#message'),
  ttl: document.querySelector('#ttl'),
  packetHex: document.querySelector('#packetHex'),
  packetState: document.querySelector('#packetState'),
  packetId: document.querySelector('#packetId'),
  packetTtl: document.querySelector('#packetTtl'),
  packetBytes: document.querySelector('#packetBytes'),
  copyButton: document.querySelector('#copyButton'),
  scanHex: document.querySelector('#scanHex'),
  injectButton: document.querySelector('#injectButton'),
  sampleButton: document.querySelector('#sampleButton'),
  activity: document.querySelector('#activity'),
  clearButton: document.querySelector('#clearButton'),
  cacheCount: document.querySelector('#cacheCount')
};

function bytesToHex(bytes) {
  return [...bytes].map((byte) => byte.toString(16).padStart(2, '0')).join('');
}

function hexToBytes(hex) {
  const clean = hex.replace(/\s+/g, '').toLowerCase();
  if (!clean || clean.length % 2 !== 0 || !/^[0-9a-f]+$/.test(clean)) {
    throw new Error('Enter an even-length hexadecimal packet.');
  }
  return Uint8Array.from(clean.match(/.{2}/g), (pair) => Number.parseInt(pair, 16));
}

function encodePacket(id, ttl, payload) {
  const bytes = new Uint8Array(HEADER_BYTES + encoder.encode(payload).length);
  const view = new DataView(bytes.buffer);
  view.setUint32(0, id);
  bytes[4] = ttl;
  bytes.set(encoder.encode(payload), HEADER_BYTES);
  return bytes;
}

function decodePacket(bytes) {
  if (bytes.length < HEADER_BYTES) throw new Error('Packet is shorter than the 5-byte header.');
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const ttl = bytes[4];
  if (ttl > MAX_TTL) throw new Error('TTL must be between 0 and 127.');
  return { id: view.getUint32(0), ttl, payload: decoder.decode(bytes.slice(HEADER_BYTES)) };
}

function addActivity(message, type = '') {
  const empty = elements.activity.querySelector('.empty-state');
  if (empty) empty.remove();
  const item = document.createElement('li');
  item.className = `activity-item ${type}`;
  const text = document.createElement('span');
  text.textContent = message;
  const time = document.createElement('time');
  time.textContent = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
  item.append(text, time);
  elements.activity.prepend(item);
}

function showPacket(bytes, state = 'Broadcast') {
  const packet = decodePacket(bytes);
  elements.packetHex.textContent = bytesToHex(bytes);
  elements.packetState.textContent = state;
  elements.packetState.classList.add('active');
  elements.packetId.textContent = packet.id;
  elements.packetTtl.textContent = packet.ttl;
  elements.packetBytes.textContent = encoder.encode(packet.payload).length;
  elements.copyButton.disabled = false;
  return packet;
}

function updateCache() {
  elements.cacheCount.textContent = `${seenIds.size} packet${seenIds.size === 1 ? '' : 's'}`;
}

function processPacket(bytes, source) {
  const packet = showPacket(bytes, source === 'scan' ? 'Received' : 'Broadcast');
  if (seenIds.has(packet.id)) {
    addActivity(`Duplicate ${packet.id} ignored`, 'warning');
    return;
  }
  seenIds.add(packet.id);
  updateCache();
  if (source === 'scan') addActivity(`SOS ${packet.id} received: ${packet.payload}`);
  if (packet.ttl > 0) addActivity(`Relaying ${packet.id} with TTL ${packet.ttl - 1}`, 'relay');
  else addActivity(`SOS ${packet.id} delivered; relay limit reached`);
}

elements.form.addEventListener('submit', (event) => {
  event.preventDefault();
  const message = elements.message.value.trim();
  const ttl = Number(elements.ttl.value);
  if (!message || !Number.isInteger(ttl) || ttl < 0 || ttl > MAX_TTL) return;
  const id = Math.floor(Math.random() * 0xffffffff);
  const bytes = encodePacket(id, ttl, message);
  processPacket(bytes, 'origin');
  addActivity(`SOS ${id} broadcast from this mobile`, 'relay');
});

elements.injectButton.addEventListener('click', () => {
  try {
    processPacket(hexToBytes(elements.scanHex.value), 'scan');
  } catch (error) {
    addActivity(error.message, 'warning');
  }
});

elements.sampleButton.addEventListener('click', () => {
  elements.scanHex.value = bytesToHex(encodePacket(42, 2, 'Need assistance'));
  elements.scanHex.focus();
});

elements.copyButton.addEventListener('click', async () => {
  await navigator.clipboard.writeText(elements.packetHex.textContent);
  addActivity('Packet hex copied to clipboard');
});

elements.clearButton.addEventListener('click', () => {
  elements.activity.innerHTML = '<li class="empty-state">No mesh events yet.</li>';
});
