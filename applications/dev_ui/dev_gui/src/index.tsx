import React from 'react';
import ReactDOM from 'react-dom';
import App from './App';
import './bootstrap.css'
import './index.css';
import process from 'process';

process.title = 'dev_gui';

ReactDOM.render(
  <App />,
  document.getElementById('root')
);
