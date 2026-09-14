/** 该模块仅适用于 UE WebView2 插件通讯 */

interface Window {
  chrome: {
    webview: {
      addEventListener(type: 'message', listener: (ev: MessageEvent) => void, useCapture: boolean): void
      postMessage(message: string, uid?: string): void
    }
  }
  getMessage(message: string): void
}

type MessageCallback = (message: string) => void

/** 回调函数映射 */
const handlers: Map<string, MessageCallback> = new Map()
let host: Element = document.body
/** 视角移动状态 */
const movements = {
  forward: 0,
  right: 0,
  up: 0,
}
/** 视角移动相关键位 */
const MOVEMENT_KEYS = ['a', 'd', 'e', 'q', 's', 'w']
/** 视角复位时间戳 */
let resetStamp = 0

function emitCustomEvent(detail: string) {
  const event = new CustomEvent('ue', { detail })
  host.dispatchEvent(event)
}

function generateUID() {
  const len = 8
  let radix = 16
  const chars =
    '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'.split('')
  const uuid = []
  radix = radix || chars.length

  for (let i = 0; i < len; i++) {
    uuid[i] = chars[0 | (Math.random() * radix)]
  }

  return uuid.join('')
}

export function listen(
  name: string,
  callback: (evt: CustomEvent<string>) => void
) {
  host.addEventListener(name as keyof ElementEventMap, callback as EventListener)
}

function onKeyDown(ev: KeyboardEvent) {
  // 检查是否为全局输入
  const tagret = ev.target as Element
  if (tagret.tagName !== 'BODY') {
    return
  }
  // 视角复位
  if (ev.key === '') {
    resetStamp = Date.now()
    return
  }
  // 如果不是视角移动相关键位，直接发送键位到 UE
  if (!MOVEMENT_KEYS.includes(ev.key)) {
    send2ue({
      type: 'keydown',
      key: ev.key,
    })
    return
  }
  // 剔除重复键位
  if (ev.repeat) {
    return
  }
  // 更新视角移动状态
  switch (ev.key) {
    case 'a': {
      movements.right -= 1
      break
    }
    case 'd': {
      movements.right += 1
      break
    }
    case 'e': {
      movements.up += 1
      break
    }
    case 'q': {
      movements.up -= 1
      break
    }
    case 's': {
      movements.forward -= 1
      break
    }
    case 'w': {
      movements.forward += 1
      break
    }
  }
  // 发送视角移动事件到 UE
  send2ue({
    type: 'cameramove',
    key: `(X=${movements.right},Y=${movements.forward},Z=${movements.up})`,
  })
}

function onKeyUp(ev: KeyboardEvent) {
  // 检查是否为全局输入
  const tagret = ev.target as Element
  if (tagret.tagName !== 'BODY') {
    return
  }
  // 视角复位
  if (ev.key === '') {
    send2ue({
      type: 'camerareset',
      key: Date.now() - resetStamp,
    })
    return
  }
  // 如果不是视角移动相关键位，直接发送键位到 UE
  if (!MOVEMENT_KEYS.includes(ev.key)) {
    send2ue({
      type: 'keyup',
      key: ev.key,
    })
    return
  }
  // 更新视角移动状态
  switch (ev.key) {
    case 'a': {
      movements.right += 1
      break
    }
    case 'd': {
      movements.right -= 1
      break
    }
    case 'e': {
      movements.up -= 1
      break
    }
    case 'q': {
      movements.up += 1
      break
    }
    case 's': {
      movements.forward += 1
      break
    }
    case 'w': {
      movements.forward -= 1
      break
    }
  }
  // 发送视角移动事件到 UE
  send2ue({
    type: 'cameramove',
    key: `(X=${movements.right},Y=${movements.forward},Z=${movements.up})`,
  })
}

function onMessage(message: string, uid?: string) {
  // 检查回调 UID
  if (!uid) {
    emitCustomEvent(message)
    return
  }
  // 检查回调函数
  const callback = handlers.get(uid)
  if (!callback) {
    return
  }
  // 执行后销毁回调函数
  callback(message)
  handlers.delete(uid)
}

let prevCursor = 'none'
function onMouseMove(ev: MouseEvent) {
  // 获取鼠标坐标下的 Element
  const element = document.elementFromPoint(ev.x, ev.y)
  if (!element) {
    return
  }
  // 获取当前元素的样式
  const style = window.getComputedStyle(element)
  // 鼠标光标样式为 auto 且 pointer-events 为 all 时，自动矫正为 text
  const cursor =
    style.pointerEvents === 'all' && style.cursor === 'auto'
      ? 'text'
      : style.cursor
  // 光标样式跟记录的一样，提前中断
  if (cursor === prevCursor) {
    return
  }
  prevCursor = cursor
  // 发送光标样式到 UE
  send2ue({
    cursor,
    type: 'cursor',
    x: ev.x,
    y: ev.y,
  })
}

export function registerEventHost(el: Element) {
  host = el
}

export function sendMessage(
  message: unknown,
  callback: MessageCallback
) {
  // 运行环境检查
  if (!window?.chrome?.webview) {
    return
  }
  // 设置回调
  handlers.set(generateUID(), callback)
  // 发送消息
  send2ue(message)
}

function send2ue(message: unknown) {
  switch (typeof message) {
    case 'boolean': {
      window.chrome.webview.postMessage(message.toString())
      break
    }
    case 'number': {
      window.chrome.webview.postMessage(message.toFixed(6))
      break
    }
    case 'string': {
      window.chrome.webview.postMessage(message)
      break
    }
    case 'object': {
      window.chrome.webview.postMessage(JSON.stringify(message))
      break
    }
    case 'string': {
      window.chrome.webview.postMessage(message)
      break
    }
  }
}

export function unlisten(
  name: string,
  callback: (evt: CustomEvent<string>) => void
) {
  host.removeEventListener(name as keyof ElementEventMap, callback as EventListener)
}

export function useWebView2() {
  // 运行环境检查
  if (!window?.chrome?.webview) {
    return
  }
  // 设置接收 UE 消息的方法
  window.getMessage = onMessage
  console.log('[cve] bind with edge webview2 runtime!')
  // 交互转发
  window.addEventListener('keydown', onKeyDown)
  window.addEventListener('keyup', onKeyUp)
  window.addEventListener('mousemove', onMouseMove)
}
