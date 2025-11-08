const firebaseConfig = {
  apiKey: "AIzaSyB3Hm969BjdLhV-2i1auDazDejCLemAe34",
  authDomain: "project1-a91b7.firebaseapp.com",
  databaseURL: "https://project1-a91b7-default-rtdb.firebaseio.com",
  projectId: "project1-a91b7",
  storageBucket: "project1-a91b7.firebasestorage.app",
  messagingSenderId: "273865965692",
  appId: "1:273865965692:web:2f24df6b13c8fd917949f9",
  measurementId: "G-5ZH17ZF55E"
};

firebase.initializeApp(firebaseConfig);
const database = firebase.database();

const elements = {
  temp: document.getElementById('nhietdo'),
  humi: document.getElementById('doam'),
  time: document.getElementById('time'),
  alarmDay: [],
  alarmMonth: [],
  alarmHour: [],
  alarmMinute: [],
  alarmEnable: []
};

for (let i = 1; i <= 5; i++) {
  elements.alarmDay[i] = document.getElementById(`alarmDay${i}`);
  elements.alarmMonth[i] = document.getElementById(`alarmMonth${i}`);
  elements.alarmHour[i] = document.getElementById(`alarmHour${i}`);
  elements.alarmMinute[i] = document.getElementById(`alarmMinute${i}`);
  elements.alarmEnable[i] = document.getElementById(`alarmEnable${i}`);
}

let sensorData = {
  temp: [],
  humi: [],
  timestamps: []
};

let alarms = Array(5).fill(null);

const ctxTemp = document.getElementById('tempChart').getContext('2d');
const ctxHumidity = document.getElementById('humidityChart').getContext('2d');

const tempChart = new Chart(ctxTemp, {
  type: 'line',
  data: {
    labels: sensorData.timestamps,
    datasets: [{
      label: 'Temperature (°C)',
      data: sensorData.temp,
      borderColor: '#ff6b6b',
      backgroundColor: 'rgba(255, 107, 107, 0.2)',
      fill: true,
      tension: 0.3
    }]
  },
  options: {
    responsive: true,
    plugins: {
      tooltip: { enabled: true },
      legend: { display: true }
    },
    scales: {
      x: { 
        title: { display: false }, 
        ticks: { display: false } // Ẩn nhãn trục x
      },
      y: { title: { display: true, text: 'Temperature (°C)' }, beginAtZero: false }
    }
  }
});

const humidityChart = new Chart(ctxHumidity, {
  type: 'line',
  data: {
    labels: sensorData.timestamps,
    datasets: [{
      label: 'Humidity (%)',
      data: sensorData.humi,
      borderColor: '#4dabf7',
      backgroundColor: 'rgba(77, 171, 247, 0.2)',
      fill: true,
      tension: 0.3
    }]
  },
  options: {
    responsive: true,
    plugins: {
      tooltip: { enabled: true },
      legend: { display: true }
    },
    scales: {
      x: { 
        title: { display: false }, 
        ticks: { display: false } // Ẩn nhãn trục x
      },
      y: { title: { display: true, text: 'Humidity (%)' }, beginAtZero: false }
    }
  }
});

function showToast(message, type = 'success') {
  Toastify({
    text: message,
    duration: 3000,
    gravity: 'top',
    position: 'right',
    backgroundColor: type === 'success' ? '#2ecc71' : '#e74c3c',
    stopOnFocus: true
  }).showToast();
}

function initializeData() {
  database.ref('.info/connected').on('value', snap => {
    console.log(snap.val() ? 'Connected to Firebase' : 'Disconnected from Firebase');
  });

  database.ref('esp8266_data/sensors').on('value', snap => {
    try {
      const data = snap.val() || { temperature: 0.0, humidity: 0.0, timestamp: 0 };
      const timestamp = data.timestamp * 1000;
      const date = new Date(timestamp);
      const timeStr = date.toLocaleTimeString();

      sensorData.timestamps.push(timeStr);
      sensorData.temp.push(data.temperature || 0.0);
      sensorData.humi.push(data.humidity || 0.0);

      if (sensorData.timestamps.length > 10) {
        sensorData.timestamps.shift();
        sensorData.temp.shift();
        sensorData.humi.shift();
      }

      updateDisplay(data);
      updateCharts();
    } catch (error) {
      console.error('Error processing sensor data:', error);
      showToast('Error fetching sensor data', 'error');
    }
  }, error => {
    console.error('Error listening to sensor data:', error);
    showToast('Sensor data connection failed', 'error');
  });

  database.ref('esp8266_data/alarms').on('value', snap => {
    try {
      alarms = Array(5).fill(null);
      const alarmData = snap.val() || {};

      for (let i = 1; i <= 5; i++) {
        const alarm = alarmData[`alarm${i}`];
        const card = document.getElementById(`alarmCard${i}`);
        if (alarm) {
          alarms[i - 1] = {
            id: i.toString(),
            day: alarm.day || 1,
            month: alarm.month || 1,
            hour: alarm.hour || 0,
            minute: alarm.minute || 0,
            enabled: alarm.enabled !== undefined ? alarm.enabled : false
          };
          elements.alarmDay[i].textContent = alarm.day.toString().padStart(2, '0');
          elements.alarmMonth[i].textContent = alarm.month.toString().padStart(2, '0');
          elements.alarmHour[i].textContent = alarm.hour.toString().padStart(2, '0');
          elements.alarmMinute[i].textContent = alarm.minute.toString().padStart(2, '0');
          elements.alarmEnable[i].checked = alarm.enabled;
          card.classList.toggle('active', alarm.enabled);
        } else {
          elements.alarmDay[i].textContent = "01";
          elements.alarmMonth[i].textContent = "01";
          elements.alarmHour[i].textContent = "00";
          elements.alarmMinute[i].textContent = "00";
          elements.alarmEnable[i].checked = false;
          card.classList.remove('active');
        }
      }
    } catch (error) {
      console.error('Error processing alarm data:', error);
      showToast('Error fetching alarm data', 'error');
    }
  }, error => {
    console.error('Error listening to alarms:', error);
    showToast('Alarm data connection failed', 'error');
  });

  database.ref('esp8266_data/sensors').once('value', snap => {
    if (!snap.exists()) {
      database.ref('esp8266_data/sensors').set({
        temperature: 0.0,
        humidity: 0.0,
        timestamp: Math.floor(Date.now() / 1000)
      }).then(() => console.log('Initialized default sensor data'))
        .catch(error => console.error('Error initializing sensor data:', error));
    }
  });

  database.ref('esp8266_data/alarms').once('value', snap => {
    if (!snap.exists()) {
      const defaultAlarms = {};
      for (let i = 1; i <= 5; i++) {
        defaultAlarms[`alarm${i}`] = {
          day: 1,
          month: 1,
          hour: 0,
          minute: 0,
          enabled: false
        };
      }
      database.ref('esp8266_data/alarms').set(defaultAlarms)
        .then(() => console.log('Initialized default alarms'))
        .catch(error => console.error('Error initializing alarms:', error));
    }
  });
}

function updateDisplay(data) {
  const temp = isNaN(data.temperature) ? 0.0 : data.temperature.toFixed(1);
  const humi = isNaN(data.humidity) ? 0.0 : data.humidity.toFixed(1);
  elements.temp.textContent = `${temp} °C`;
  elements.humi.textContent = `${humi} %`;
  elements.temp.style.color = temp > 30 ? '#e74c3c' : temp < 20 ? '#3498db' : '#2c3e50';
  elements.humi.style.color = humi > 70 ? '#e74c3c' : humi < 30 ? '#3498db' : '#2c3e50';
}

function updateCharts() {
  tempChart.data.labels = sensorData.timestamps;
  tempChart.data.datasets[0].data = sensorData.temp;
  tempChart.update();

  humidityChart.data.labels = sensorData.timestamps;
  humidityChart.data.datasets[0].data = sensorData.humi;
  humidityChart.update();
}

function saveAlarm(alarmIndex) {
  const newAlarm = {
    day: parseInt(elements.alarmDay[alarmIndex].textContent) || 1,
    month: parseInt(elements.alarmMonth[alarmIndex].textContent) || 1,
    hour: parseInt(elements.alarmHour[alarmIndex].textContent) || 0,
    minute: parseInt(elements.alarmMinute[alarmIndex].textContent) || 0,
    enabled: elements.alarmEnable[alarmIndex].checked
  };

  newAlarm.day = Math.max(1, Math.min(31, newAlarm.day));
  newAlarm.month = Math.max(1, Math.min(12, newAlarm.month));
  newAlarm.hour = Math.max(0, Math.min(23, newAlarm.hour));
  newAlarm.minute = Math.max(0, Math.min(59, newAlarm.minute));

  database.ref(`esp8266_data/alarms/alarm${alarmIndex}`).set(newAlarm)
    .then(() => {
      console.log(`Alarm ${alarmIndex} saved successfully`);
      showToast(`Alarm ${alarmIndex} saved!`);
      document.getElementById(`alarmCard${alarmIndex}`).classList.toggle('active', newAlarm.enabled);
    })
    .catch(error => {
      console.error(`Error saving alarm ${alarmIndex}:`, error);
      showToast(`Error saving alarm ${alarmIndex}!`, 'error');
    });
}

function deleteAlarm(alarmIndex) {
  const defaultAlarm = {
    day: 1,
    month: 1,
    hour: 0,
    minute: 0,
    enabled: false
  };

  database.ref(`esp8266_data/alarms/alarm${alarmIndex}`).set(defaultAlarm)
    .then(() => {
      console.log(`Alarm ${alarmIndex} deleted successfully`);
      showToast(`Alarm ${alarmIndex} deleted!`);
      document.getElementById(`alarmCard${alarmIndex}`).classList.remove('active');
    })
    .catch(error => {
      console.error(`Error deleting alarm ${alarmIndex}:`, error);
      showToast(`Error deleting alarm ${alarmIndex}!`, 'error');
    });
}

function toggleAlarmEnable(alarmIndex) {
  const isEnabled = elements.alarmEnable[alarmIndex].checked;
  database.ref(`esp8266_data/alarms/alarm${alarmIndex}`).update({ enabled: isEnabled })
    .then(() => {
      console.log(`Alarm ${alarmIndex} enable state updated: ${isEnabled}`);
      document.getElementById(`alarmCard${alarmIndex}`).classList.toggle('active', isEnabled);
    })
    .catch(error => {
      console.error(`Error updating alarm ${alarmIndex} enable state:`, error);
      showToast(`Error updating alarm ${alarmIndex} enable state!`, 'error');
    });
}

function adjustAlarm(type, delta, alarmIndex) {
  let valueElement, minValue, maxValue;
  switch (type) {
    case 'day':
      valueElement = elements.alarmDay[alarmIndex];
      minValue = 1;
      maxValue = 31;
      break;
    case 'month':
      valueElement = elements.alarmMonth[alarmIndex];
      minValue = 1;
      maxValue = 12;
      break;
    case 'hour':
      valueElement = elements.alarmHour[alarmIndex];
      minValue = 0;
      maxValue = 23;
      break;
    case 'minute':
      valueElement = elements.alarmMinute[alarmIndex];
      minValue = 0;
      maxValue = 59;
      break;
    default:
      return;
  }

  let currentValue = parseInt(valueElement.textContent) || 0;
  let newValue = currentValue + delta;
  newValue = Math.max(minValue, Math.min(maxValue, newValue));
  valueElement.textContent = newValue.toString().padStart(2, '0');
}

function updateClock() {
  const now = new Date();
  elements.time.textContent = now.toLocaleTimeString('en-US', { hour12: false });
}

window.onload = () => {
  initializeData();
  setInterval(updateClock, 1000);
  updateClock();
};