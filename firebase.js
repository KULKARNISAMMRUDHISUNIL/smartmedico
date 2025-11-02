// Import SDKs
import { initializeApp } from "https://www.gstatic.com/firebasejs/11.0.1/firebase-app.js";
import { getFirestore } from "https://www.gstatic.com/firebasejs/11.0.1/firebase-firestore.js";

// Firebase config
const firebaseConfig = {
  apiKey: "AIzaSyBI-DHfeIH0cGMTQx60SCk5SKzjWFH3RZs",
  authDomain: "smart-health-monitoring-d1b63.firebaseapp.com",
  projectId: "smart-health-monitoring-d1b63",
  storageBucket: "smart-health-monitoring-d1b63.appspot.com", // ✅ fixed
  messagingSenderId: "292788296841",
  appId: "1:292788296841:web:5bd140a662f0ce64e1fa58",
  measurementId: "G-LGR33JW2RX"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
export const db = getFirestore(app);
